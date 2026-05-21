#[link(name = "jnigraphics")]
extern "C" {}

use imagequant::{Attributes, RGBA};
use jni::objects::{JClass, JObject};
use jni::sys::{jboolean, jbyteArray, jdouble, jfloat, jint, JNI_TRUE};
use jni::JNIEnv;
use png::{AdaptiveFilterType, BitDepth, ColorType, Compression, FilterType};

#[repr(C)]
#[derive(Debug)]
struct AndroidBitmapInfo {
    width: u32,
    height: u32,
    stride: u32,
    format: i32,
    flags: u32,
}

const ANDROID_BITMAP_FORMAT_RGBA_8888: i32 = 1;

extern "C" {
    fn AndroidBitmap_getInfo(
        env: *mut jni::sys::JNIEnv,
        bitmap: jni::sys::jobject,
        info: *mut AndroidBitmapInfo,
    ) -> i32;

    fn AndroidBitmap_lockPixels(
        env: *mut jni::sys::JNIEnv,
        bitmap: jni::sys::jobject,
        pixels: *mut *mut std::ffi::c_void,
    ) -> i32;

    fn AndroidBitmap_unlockPixels(env: *mut jni::sys::JNIEnv, bitmap: jni::sys::jobject) -> i32;
}

struct BitmapBuffer {
    width: u32,
    height: u32,
    pixels: Vec<RGBA>,
}

fn throw(env: &mut JNIEnv, msg: &str) {
    let _ = env.throw_new("java/lang/RuntimeException", msg);
}

fn jbool(value: jboolean) -> bool {
    value == JNI_TRUE
}

fn normalized_gamma(value: jdouble) -> f64 {
    let value = value as f64;
    if value.is_finite() && value > 0.0 {
        value
    } else {
        0.0
    }
}

fn compression(value: jint) -> Compression {
    match value {
        1 => Compression::Fast,
        2 => Compression::Best,
        _ => Compression::Default,
    }
}

fn filter(value: jint) -> Option<FilterType> {
    match value {
        0 => Some(FilterType::NoFilter),
        1 => Some(FilterType::Sub),
        2 => Some(FilterType::Up),
        3 => Some(FilterType::Avg),
        4 => Some(FilterType::Paeth),
        _ => None,
    }
}

fn selected_bit_depth(requested: jint, palette_len: usize) -> (BitDepth, u8) {
    let required = if palette_len <= 2 {
        1
    } else if palette_len <= 4 {
        2
    } else if palette_len <= 16 {
        4
    } else {
        8
    };
    let requested = match requested {
        1 | 2 | 4 | 8 => requested as u8,
        _ => required,
    };
    let bits = requested.max(required);
    let depth = match bits {
        1 => BitDepth::One,
        2 => BitDepth::Two,
        4 => BitDepth::Four,
        _ => BitDepth::Eight,
    };
    (depth, bits)
}

unsafe fn read_bitmap(env: &mut JNIEnv, bitmap: &JObject) -> Result<BitmapBuffer, String> {
    let raw_env = env.get_raw();
    let raw_bitmap = bitmap.as_raw();
    let mut info = AndroidBitmapInfo {
        width: 0,
        height: 0,
        stride: 0,
        format: 0,
        flags: 0,
    };

    if AndroidBitmap_getInfo(raw_env, raw_bitmap, &mut info) < 0 {
        return Err("AndroidBitmap_getInfo failed".to_string());
    }
    if info.format != ANDROID_BITMAP_FORMAT_RGBA_8888 {
        return Err(format!("Unsupported bitmap format: {}", info.format));
    }

    let mut pixels_ptr: *mut std::ffi::c_void = std::ptr::null_mut();
    if AndroidBitmap_lockPixels(raw_env, raw_bitmap, &mut pixels_ptr) < 0 {
        return Err("AndroidBitmap_lockPixels failed".to_string());
    }

    let width = info.width as usize;
    let height = info.height as usize;
    let row_bytes = width * 4;
    let stride = info.stride as usize;
    let src = pixels_ptr as *const u8;
    let mut pixels = Vec::with_capacity(width * height);

    for y in 0..height {
        let row = std::slice::from_raw_parts(src.add(y * stride), row_bytes);
        for px in row.chunks_exact(4) {
            pixels.push(RGBA::new(px[0], px[1], px[2], px[3]));
        }
    }

    if AndroidBitmap_unlockPixels(raw_env, raw_bitmap) < 0 {
        return Err("AndroidBitmap_unlockPixels failed".to_string());
    }

    Ok(BitmapBuffer {
        width: info.width,
        height: info.height,
        pixels,
    })
}

fn build_attributes(
    max_colors: jint,
    min_quality: jint,
    target_quality: jint,
    speed: jint,
    min_posterization: jint,
    last_index_transparent: jboolean,
) -> Result<Attributes, String> {
    let mut attr = Attributes::new();
    let min_quality = min_quality.clamp(0, 100) as u8;
    let target_quality = target_quality.clamp(min_quality as jint, 100) as u8;

    attr.set_max_colors(max_colors.clamp(2, 256) as u32)
        .map_err(|e| format!("set_max_colors failed: {e:?}"))?;
    attr.set_quality(min_quality, target_quality)
        .map_err(|e| format!("set_quality failed: {e:?}"))?;
    attr.set_speed(speed.clamp(1, 10))
        .map_err(|e| format!("set_speed failed: {e:?}"))?;
    attr.set_min_posterization(min_posterization.clamp(0, 4) as u8)
        .map_err(|e| format!("set_min_posterization failed: {e:?}"))?;
    attr.set_last_index_transparent(jbool(last_index_transparent));

    Ok(attr)
}

fn pack_indices(indices: &[u8], width: usize, height: usize, bits: u8) -> Vec<u8> {
    if bits == 8 {
        return indices.to_vec();
    }

    let row_len = (width * bits as usize + 7) / 8;
    let mut packed = vec![0u8; row_len * height];
    let mask = (1u8 << bits) - 1;

    for y in 0..height {
        let src_row = &indices[y * width..(y + 1) * width];
        let dst_row = &mut packed[y * row_len..(y + 1) * row_len];
        for (x, &index) in src_row.iter().enumerate() {
            let bit_offset = x * bits as usize;
            let byte_index = bit_offset / 8;
            let shift = 8 - bits - (bit_offset % 8) as u8;
            dst_row[byte_index] |= (index & mask) << shift;
        }
    }

    packed
}

fn encode_indexed_png(
    width: u32,
    height: u32,
    palette: &[RGBA],
    indices: &[u8],
    bit_depth: jint,
    png_compression: jint,
    png_filter: jint,
    adaptive_filter: jboolean,
) -> Result<Vec<u8>, String> {
    let mut palette_rgb = Vec::with_capacity(palette.len() * 3);
    let mut palette_alpha = Vec::with_capacity(palette.len());
    for color in palette {
        palette_rgb.extend_from_slice(&[color.r, color.g, color.b]);
        palette_alpha.push(color.a);
    }
    while palette_alpha.last() == Some(&255) {
        palette_alpha.pop();
    }

    let (depth, bits) = selected_bit_depth(bit_depth, palette.len());
    let data = pack_indices(indices, width as usize, height as usize, bits);
    let mut output = Vec::new();
    {
        let mut encoder = png::Encoder::new(&mut output, width, height);
        encoder.set_color(ColorType::Indexed);
        encoder.set_depth(depth);
        encoder.set_palette(palette_rgb);
        if !palette_alpha.is_empty() {
            encoder.set_trns(palette_alpha);
        }
        encoder.set_compression(compression(png_compression));
        if jbool(adaptive_filter) || png_filter == 5 {
            encoder.set_adaptive_filter(AdaptiveFilterType::Adaptive);
        } else if let Some(filter) = filter(png_filter) {
            encoder.set_filter(filter);
        }

        let mut writer = encoder.write_header().map_err(|e| e.to_string())?;
        writer.write_image_data(&data).map_err(|e| e.to_string())?;
    }

    Ok(output)
}

#[no_mangle]
#[allow(clippy::too_many_arguments)]
pub unsafe extern "system" fn Java_com_t8rin_trickle_ImageQuant_compressBitmapNative(
    mut env: JNIEnv,
    _class: JClass,
    bitmap: JObject,
    max_colors: jint,
    min_quality: jint,
    target_quality: jint,
    speed: jint,
    min_posterization: jint,
    dithering_level: jfloat,
    input_gamma: jdouble,
    output_gamma: jdouble,
    last_index_transparent: jboolean,
    bit_depth: jint,
    png_compression: jint,
    png_filter: jint,
    adaptive_filter: jboolean,
) -> jbyteArray {
    let buffer = match read_bitmap(&mut env, &bitmap) {
        Ok(buffer) => buffer,
        Err(e) => {
            throw(&mut env, &e);
            return std::ptr::null_mut();
        }
    };

    let attr = match build_attributes(
        max_colors,
        min_quality,
        target_quality,
        speed,
        min_posterization,
        last_index_transparent,
    ) {
        Ok(attr) => attr,
        Err(e) => {
            throw(&mut env, &e);
            return std::ptr::null_mut();
        }
    };

    let mut image = match attr.new_image(
        buffer.pixels,
        buffer.width as usize,
        buffer.height as usize,
        normalized_gamma(input_gamma),
    ) {
        Ok(image) => image,
        Err(e) => {
            throw(&mut env, &format!("new_image failed: {e:?}"));
            return std::ptr::null_mut();
        }
    };

    let mut result = match attr.quantize(&mut image) {
        Ok(result) => result,
        Err(e) => {
            throw(&mut env, &format!("quantize failed: {e:?}"));
            return std::ptr::null_mut();
        }
    };

    let dithering_level = (dithering_level as f32).clamp(0.0, 1.0);
    if let Err(e) = result.set_dithering_level(dithering_level) {
        throw(&mut env, &format!("set_dithering_level failed: {e:?}"));
        return std::ptr::null_mut();
    }
    let output_gamma = normalized_gamma(output_gamma);
    if output_gamma > 0.0 {
        if let Err(e) = result.set_output_gamma(output_gamma) {
            throw(&mut env, &format!("set_output_gamma failed: {e:?}"));
            return std::ptr::null_mut();
        }
    }

    let (palette, indices) = match result.remapped(&mut image) {
        Ok(remapped) => remapped,
        Err(e) => {
            throw(&mut env, &format!("remap failed: {e:?}"));
            return std::ptr::null_mut();
        }
    };

    let png = match encode_indexed_png(
        buffer.width,
        buffer.height,
        &palette,
        &indices,
        bit_depth,
        png_compression,
        png_filter,
        adaptive_filter,
    ) {
        Ok(png) => png,
        Err(e) => {
            throw(&mut env, &e);
            return std::ptr::null_mut();
        }
    };

    let out = match env.new_byte_array(png.len() as i32) {
        Ok(arr) => arr,
        Err(e) => {
            throw(&mut env, &e.to_string());
            return std::ptr::null_mut();
        }
    };
    let signed: Vec<i8> = png.into_iter().map(|b| b as i8).collect();
    if env.set_byte_array_region(&out, 0, &signed).is_err() {
        throw(&mut env, "set_byte_array_region failed");
        return std::ptr::null_mut();
    }
    out.into_raw()
}
