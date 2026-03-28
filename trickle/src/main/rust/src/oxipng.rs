use jni::objects::{JClass, JObject};
use jni::sys::{jbyteArray, jint, jlong, jboolean, JNI_TRUE};
use jni::JNIEnv;
use oxipng::{Options, StripChunks};

#[link(name = "jnigraphics")]
extern "C" {}

// ---------------------------------------------------------------------------
// android/bitmap.h bindings (libjnigraphics)
// ---------------------------------------------------------------------------

#[repr(C)]
#[derive(Debug)]
struct AndroidBitmapInfo {
    width:  u32,
    height: u32,
    stride: u32,
    format: i32,
    flags:  u32,
}

const ANDROID_BITMAP_FORMAT_RGBA_8888: i32 = 1;
const ANDROID_BITMAP_FORMAT_RGB_565:   i32 = 4;

extern "C" {
    fn AndroidBitmap_getInfo(
        env:    *mut jni::sys::JNIEnv,
        bitmap: jni::sys::jobject,
        info:   *mut AndroidBitmapInfo,
    ) -> i32;

    fn AndroidBitmap_lockPixels(
        env:    *mut jni::sys::JNIEnv,
        bitmap: jni::sys::jobject,
        pixels: *mut *mut std::ffi::c_void,
    ) -> i32;

    fn AndroidBitmap_unlockPixels(
        env:    *mut jni::sys::JNIEnv,
        bitmap: jni::sys::jobject,
    ) -> i32;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

fn throw(env: &mut JNIEnv, msg: &str) {
    let _ = env.throw_new("java/lang/RuntimeException", msg);
}

fn build_options(
    level:      jint,
    strip_all:  jboolean,
    fix_errors: jboolean,
    force:      jboolean,
    interlace:  jint,
    timeout_ms: jlong,
) -> Options {
    // Options::from_preset даёт правильные дефолты для каждого уровня,
    // включая нужные фильтры и deflater — не надо задавать вручную
    let mut opts = Options::from_preset(level.clamp(0, 6) as u8);

    opts.strip      = if strip_all == JNI_TRUE { StripChunks::All } else { StripChunks::Safe };
    opts.fix_errors = fix_errors == JNI_TRUE;
    opts.force      = force == JNI_TRUE;
    opts.interlace  = match interlace {
        0 => Some(false),
        1 => Some(true),
        _ => None,
    };
    if timeout_ms > 0 {
        opts.timeout = Some(std::time::Duration::from_millis(timeout_ms as u64));
    }

    opts
}

fn encode_rgba_to_png(
    pixels: *const u8,
    width:  u32,
    height: u32,
    stride: u32,
) -> Result<Vec<u8>, String> {
    use std::io::Write;
    let mut buf: Vec<u8> = Vec::new();
    {
        let mut encoder = png::Encoder::new(&mut buf, width, height);
        encoder.set_color(png::ColorType::Rgba);
        encoder.set_depth(png::BitDepth::Eight);
        let mut writer = encoder.write_header().map_err(|e| e.to_string())?;
        let mut stream = writer.stream_writer().map_err(|e| e.to_string())?;
        let row_bytes    = (width * 4) as usize;
        let stride_usize = stride as usize;
        for row in 0..height as usize {
            // SAFETY: пиксели залочены AndroidBitmap_lockPixels
            let row_slice = unsafe {
                std::slice::from_raw_parts(pixels.add(row * stride_usize), row_bytes)
            };
            stream.write_all(row_slice).map_err(|e| e.to_string())?;
        }
        stream.finish().map_err(|e| e.to_string())?;
    }
    Ok(buf)
}

fn encode_rgb565_to_png(
    pixels: *const u8,
    width:  u32,
    height: u32,
    stride: u32,
) -> Result<Vec<u8>, String> {
    let mut rgba = Vec::with_capacity((width * height * 4) as usize);
    for row in 0..height as usize {
        for col in 0..width as usize {
            let offset = row * stride as usize + col * 2;
            let (lo, hi) = unsafe {
                (*pixels.add(offset) as u16, *pixels.add(offset + 1) as u16)
            };
            let p = (hi << 8) | lo;
            let r = ((p >> 11) & 0x1F) as u8;
            let g = ((p >> 5)  & 0x3F) as u8;
            let b = (p         & 0x1F) as u8;
            rgba.push((r << 3) | (r >> 2));
            rgba.push((g << 2) | (g >> 4));
            rgba.push((b << 3) | (b >> 2));
            rgba.push(255u8);
        }
    }
    encode_rgba_to_png(rgba.as_ptr(), width, height, width * 4)
}

// ---------------------------------------------------------------------------
// JNI export
// ---------------------------------------------------------------------------

#[no_mangle]
pub unsafe extern "system" fn Java_com_t8rin_trickle_Oxipng_optimizeBitmapNative(
    mut env:    JNIEnv,
    _class:     JClass,
    bitmap:     JObject,
    level:      jint,
    strip_all:  jboolean,
    fix_errors: jboolean,
    force:      jboolean,
    interlace:  jint,
    timeout_ms: jlong,
) -> jbyteArray {
    let raw_env = env.get_raw();
    let raw_bmp = bitmap.as_raw();

    let mut info = AndroidBitmapInfo { width: 0, height: 0, stride: 0, format: 0, flags: 0 };
    if AndroidBitmap_getInfo(raw_env, raw_bmp, &mut info) < 0 {
        throw(&mut env, "AndroidBitmap_getInfo failed");
        return std::ptr::null_mut();
    }

    let mut pixels_ptr: *mut std::ffi::c_void = std::ptr::null_mut();
    if AndroidBitmap_lockPixels(raw_env, raw_bmp, &mut pixels_ptr) < 0 {
        throw(&mut env, "AndroidBitmap_lockPixels failed");
        return std::ptr::null_mut();
    }

    let raw = pixels_ptr as *const u8;

    let png_result = match info.format {
        ANDROID_BITMAP_FORMAT_RGBA_8888 =>
            encode_rgba_to_png(raw, info.width, info.height, info.stride),
        ANDROID_BITMAP_FORMAT_RGB_565 =>
            encode_rgb565_to_png(raw, info.width, info.height, info.stride),
        other =>
            Err(format!("Unsupported bitmap format: {}", other)),
    };

    AndroidBitmap_unlockPixels(raw_env, raw_bmp);

    let optimized = match png_result {
        Err(e)  => { throw(&mut env, &e); return std::ptr::null_mut(); }
        Ok(png) => {
            let opts = build_options(level, strip_all, fix_errors, force, interlace, timeout_ms);
            match oxipng::optimize_from_memory(&png, &opts) {
                Err(e) => { throw(&mut env, &e.to_string()); return std::ptr::null_mut(); }
                Ok(v)  => v,
            }
        }
    };

    let out = match env.new_byte_array(optimized.len() as i32) {
        Err(e) => { throw(&mut env, &e.to_string()); return std::ptr::null_mut(); }
        Ok(v)  => v,
    };
    let signed: Vec<i8> = optimized.into_iter().map(|b| b as i8).collect();
    if env.set_byte_array_region(&out, 0, &signed).is_err() {
        throw(&mut env, "set_byte_array_region failed");
        return std::ptr::null_mut();
    }

    out.into_raw()
}