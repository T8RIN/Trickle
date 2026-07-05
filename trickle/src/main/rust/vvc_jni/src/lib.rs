#[link(name = "jnigraphics")]
unsafe extern "C" {}

use garnetash::{
    BitDepth, ChromaFormat, Cicp, ColorMetadata, ContentLightLevel, DecodedImage, EncodeConfig,
    ImageMetadata, MatrixCoefficients, Orientation, Primaries, TransferFunction, decode,
    decode_266, encode_rgba8, encode_rgba8_266, encode_rgba8_with_alpha, encode_rgba10,
    encode_rgba10_266, encode_rgba10_with_alpha, encode_rgba12, encode_rgba12_266,
    encode_rgba12_with_alpha,
};
use jni::JNIEnv;
use jni::objects::{JByteArray, JClass, JObject};
use jni::sys::{JNI_TRUE, jboolean, jbyteArray, jint};

#[repr(C)]
struct AndroidBitmapInfo {
    width: u32,
    height: u32,
    stride: u32,
    format: i32,
    flags: u32,
}

const ANDROID_BITMAP_FORMAT_RGBA_8888: i32 = 1;
const RAW_VVC: jint = 0;
const DECODE_HEADER_SIZE: usize = 31;

unsafe extern "C" {
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

fn throw(env: &mut JNIEnv, message: &str) {
    let _ = env.throw_new("java/lang/IllegalArgumentException", message);
}

fn jbool(value: jboolean) -> bool {
    value == JNI_TRUE
}

fn bit_depth(value: jint) -> Result<BitDepth, String> {
    match value {
        8 => Ok(BitDepth::Eight),
        10 => Ok(BitDepth::Ten),
        12 => Ok(BitDepth::Twelve),
        _ => Err(format!("Unsupported bit depth: {value}")),
    }
}

fn chroma(value: jint) -> Result<ChromaFormat, String> {
    match value {
        0 => Ok(ChromaFormat::Monochrome),
        1 => Ok(ChromaFormat::Yuv420),
        2 => Ok(ChromaFormat::Yuv422),
        3 => Ok(ChromaFormat::Yuv444),
        _ => Err(format!("Unsupported chroma format: {value}")),
    }
}

fn orientation(value: jint) -> Orientation {
    Orientation::from_exif(value as u16)
}

fn optional_bytes(env: &mut JNIEnv, array: &JByteArray) -> Result<Option<Vec<u8>>, String> {
    let bytes = env.convert_byte_array(array).map_err(|e| e.to_string())?;
    Ok((!bytes.is_empty()).then_some(bytes))
}

unsafe fn read_bitmap(env: &mut JNIEnv, bitmap: &JObject) -> Result<(Vec<u8>, u32, u32), String> {
    let raw_env = env.get_raw();
    let raw_bitmap = bitmap.as_raw();
    let mut info = AndroidBitmapInfo {
        width: 0,
        height: 0,
        stride: 0,
        format: 0,
        flags: 0,
    };
    if unsafe { AndroidBitmap_getInfo(raw_env, raw_bitmap, &mut info) } < 0 {
        return Err("AndroidBitmap_getInfo failed".to_string());
    }
    if info.format != ANDROID_BITMAP_FORMAT_RGBA_8888 {
        return Err(format!("Unsupported bitmap format: {}", info.format));
    }

    let mut pixels_ptr: *mut std::ffi::c_void = std::ptr::null_mut();
    if unsafe { AndroidBitmap_lockPixels(raw_env, raw_bitmap, &mut pixels_ptr) } < 0 {
        return Err("AndroidBitmap_lockPixels failed".to_string());
    }

    let row_bytes = info.width as usize * 4;
    let mut rgba = Vec::with_capacity(row_bytes * info.height as usize);
    for y in 0..info.height as usize {
        let row = unsafe {
            std::slice::from_raw_parts(
                (pixels_ptr as *const u8).add(y * info.stride as usize),
                row_bytes,
            )
        };
        rgba.extend_from_slice(row);
    }
    if unsafe { AndroidBitmap_unlockPixels(raw_env, raw_bitmap) } < 0 {
        return Err("AndroidBitmap_unlockPixels failed".to_string());
    }
    Ok((rgba, info.width, info.height))
}

fn widen(samples: &[u8], max: u16) -> Vec<u16> {
    samples
        .iter()
        .map(|&sample| (sample as u32 * max as u32 / 255) as u16)
        .collect()
}

fn encode_bitmap(
    rgba: &[u8],
    width: u32,
    height: u32,
    cfg: &EncodeConfig,
    container: jint,
    preserve_alpha: bool,
) -> Result<Vec<u8>, String> {
    if container == RAW_VVC && preserve_alpha {
        return Err(
            "A raw VVC stream cannot preserve alpha; use HEIF or disable alpha".to_string(),
        );
    }
    let result = match (cfg.bit_depth, container, preserve_alpha) {
        (BitDepth::Eight, RAW_VVC, _) => encode_rgba8_266(rgba, width, height, cfg),
        (BitDepth::Eight, _, false) => encode_rgba8(rgba, width, height, cfg),
        (BitDepth::Eight, _, true) => encode_rgba8_with_alpha(rgba, width, height, cfg),
        (BitDepth::Ten, RAW_VVC, _) => encode_rgba10_266(&widen(rgba, 1023), width, height, cfg),
        (BitDepth::Ten, _, false) => encode_rgba10(&widen(rgba, 1023), width, height, cfg),
        (BitDepth::Ten, _, true) => {
            encode_rgba10_with_alpha(&widen(rgba, 1023), width, height, cfg)
        }
        (BitDepth::Twelve, RAW_VVC, _) => encode_rgba12_266(&widen(rgba, 4095), width, height, cfg),
        (BitDepth::Twelve, _, false) => encode_rgba12(&widen(rgba, 4095), width, height, cfg),
        (BitDepth::Twelve, _, true) => {
            encode_rgba12_with_alpha(&widen(rgba, 4095), width, height, cfg)
        }
    };
    result.map_err(|e| format!("VVC encode failed: {e}"))
}

fn sample(data: &[u8], index: usize, bytes_per_sample: usize) -> u16 {
    if bytes_per_sample == 1 {
        data[index] as u16
    } else {
        let offset = index * 2;
        u16::from_le_bytes([data[offset], data[offset + 1]])
    }
}

fn decoded_rgba(image: &DecodedImage) -> Vec<u8> {
    let width = image.width as usize;
    let height = image.height as usize;
    let bytes_per_sample = if image.bit_depth == BitDepth::Eight {
        1
    } else {
        2
    };
    let max = image.bit_depth.max_val() as i32;
    let y_len = width * height * bytes_per_sample;
    let alpha = image.alpha_plane();
    let mut rgba = vec![0u8; width * height * 4];

    if image.chroma == ChromaFormat::Monochrome {
        for index in 0..width * height {
            let value =
                (sample(&image.planes[..y_len], index, bytes_per_sample) as i32 * 255 / max) as u8;
            let dst = index * 4;
            rgba[dst..dst + 3].fill(value);
            rgba[dst + 3] = alpha
                .map(|plane| {
                    (sample(plane.data, index, plane.bytes_per_sample) as i32 * 255 / max) as u8
                })
                .unwrap_or(255);
        }
        return rgba;
    }

    let sub_w = image.chroma.sub_w();
    let sub_h = image.chroma.sub_h();
    let chroma_width = width.div_ceil(sub_w);
    let chroma_height = height.div_ceil(sub_h);
    let chroma_len = chroma_width * chroma_height * bytes_per_sample;
    let cb = &image.planes[y_len..y_len + chroma_len];
    let cr = &image.planes[y_len + chroma_len..y_len + 2 * chroma_len];
    let neutral = 1i32 << (image.bit_depth.bits() - 1);

    for y in 0..height {
        for x in 0..width {
            let index = y * width + x;
            let chroma_index = (y / sub_h) * chroma_width + x / sub_w;
            let luma = sample(&image.planes[..y_len], index, bytes_per_sample) as i32;
            let cb = sample(cb, chroma_index, bytes_per_sample) as i32 - neutral;
            let cr = sample(cr, chroma_index, bytes_per_sample) as i32 - neutral;
            let r = luma + ((11485 * cr + 4096) >> 13);
            let g = luma - ((2819 * cb + 5850 * cr + 4096) >> 13);
            let b = luma + ((14516 * cb + 4096) >> 13);
            let dst = index * 4;
            rgba[dst] = (r.clamp(0, max) * 255 / max) as u8;
            rgba[dst + 1] = (g.clamp(0, max) * 255 / max) as u8;
            rgba[dst + 2] = (b.clamp(0, max) * 255 / max) as u8;
            rgba[dst + 3] = alpha
                .map(|plane| {
                    (sample(plane.data, index, plane.bytes_per_sample) as i32 * 255 / max) as u8
                })
                .unwrap_or(255);
        }
    }
    rgba
}

fn orientation_exif(value: Orientation) -> u8 {
    match value {
        Orientation::Normal => 1,
        Orientation::FlipH => 2,
        Orientation::Rotate180 => 3,
        Orientation::FlipV => 4,
        Orientation::Transpose => 5,
        Orientation::Rotate90 => 6,
        Orientation::Transverse => 7,
        Orientation::Rotate270 => 8,
    }
}

fn decoded_packet(image: &DecodedImage) -> Vec<u8> {
    let rgba = decoded_rgba(image);
    let cicp = image.color.cicp;
    let icc = image.color.icc.as_deref().unwrap_or_default();
    let mut output = Vec::with_capacity(DECODE_HEADER_SIZE + icc.len() + rgba.len());
    output.extend_from_slice(b"VVC1");
    output.extend_from_slice(&image.width.to_le_bytes());
    output.extend_from_slice(&image.height.to_le_bytes());
    output.push(image.chroma.idc() as u8);
    output.push(image.bit_depth.bits());
    output.push(orientation_exif(image.orientation));
    output.push(image.alpha.is_some() as u8);
    output.extend_from_slice(
        &cicp
            .map(|c| c.primaries as u16)
            .unwrap_or(u16::MAX)
            .to_le_bytes(),
    );
    output.extend_from_slice(
        &cicp
            .map(|c| c.transfer as u16)
            .unwrap_or(u16::MAX)
            .to_le_bytes(),
    );
    output.extend_from_slice(
        &cicp
            .map(|c| c.matrix as u16)
            .unwrap_or(u16::MAX)
            .to_le_bytes(),
    );
    output.push(cicp.map(|c| c.full_range).unwrap_or(false) as u8);
    output.extend_from_slice(&(icc.len() as u32).to_le_bytes());
    output.extend_from_slice(&(rgba.len() as u32).to_le_bytes());
    output.extend_from_slice(icc);
    output.extend_from_slice(&rgba);
    output
}

fn to_java_bytes(env: &mut JNIEnv, bytes: &[u8]) -> Result<jbyteArray, String> {
    env.byte_array_from_slice(bytes)
        .map(JByteArray::into_raw)
        .map_err(|e| e.to_string())
}

#[unsafe(no_mangle)]
#[allow(clippy::too_many_arguments)]
pub unsafe extern "system" fn Java_com_t8rin_trickle_VvcEncoder_encodeNative(
    mut env: JNIEnv,
    _class: JClass,
    bitmap: JObject,
    quality: jint,
    lossless: jboolean,
    chroma_value: jint,
    bit_depth_value: jint,
    threads: jint,
    rdoq: jboolean,
    aq: jboolean,
    mtt: jboolean,
    lfnst: jboolean,
    dep_quant: jboolean,
    mts: jboolean,
    dual_tree: jboolean,
    cclm: jboolean,
    deblock: jboolean,
    container: jint,
    preserve_alpha: jboolean,
    has_cicp: jboolean,
    primaries: jint,
    transfer: jint,
    matrix: jint,
    full_range: jboolean,
    icc_profile: JByteArray,
    orientation_value: jint,
    max_cll: jint,
    max_fall: jint,
    exif: JByteArray,
) -> jbyteArray {
    let result = (|| -> Result<Vec<u8>, String> {
        let (rgba, width, height) = unsafe { read_bitmap(&mut env, &bitmap) }?;
        let mut color = ColorMetadata::default();
        if jbool(has_cicp) {
            color.cicp = Some(Cicp {
                primaries: Primaries::from_u16(primaries as u16),
                transfer: TransferFunction::from_u16(transfer as u16),
                matrix: MatrixCoefficients::from_u16(matrix as u16),
                full_range: jbool(full_range),
            });
        }
        color.icc = optional_bytes(&mut env, &icc_profile)?;

        let mut metadata = ImageMetadata::new().with_orientation(orientation(orientation_value));
        if max_cll >= 0 && max_fall >= 0 {
            metadata.content_light_level = Some(ContentLightLevel::new(
                max_cll.clamp(0, u16::MAX as jint) as u16,
                max_fall.clamp(0, u16::MAX as jint) as u16,
            ));
        }
        metadata.exif = optional_bytes(&mut env, &exif)?;

        let cfg = EncodeConfig::new()
            .with_quality(quality.clamp(1, 100) as u8)
            .with_lossless(jbool(lossless))
            .with_chroma(chroma(chroma_value)?)
            .with_bit_depth(bit_depth(bit_depth_value)?)
            .with_threads(threads.max(0) as usize)
            .with_rdoq(jbool(rdoq))
            .with_aq(jbool(aq))
            .with_mtt(jbool(mtt))
            .with_lfnst(jbool(lfnst))
            .with_dep_quant(jbool(dep_quant))
            .with_mts(jbool(mts))
            .with_dual_tree(jbool(dual_tree))
            .with_cclm(jbool(cclm))
            .with_deblocking(jbool(deblock))
            .with_color(color)
            .with_metadata(metadata);
        encode_bitmap(&rgba, width, height, &cfg, container, jbool(preserve_alpha))
    })();

    match result.and_then(|bytes| to_java_bytes(&mut env, &bytes)) {
        Ok(array) => array,
        Err(error) => {
            throw(&mut env, &error);
            std::ptr::null_mut()
        }
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "system" fn Java_com_t8rin_trickle_VvcDecoder_decodeNative(
    mut env: JNIEnv,
    _class: JClass,
    encoded: JByteArray,
    container: jint,
) -> jbyteArray {
    let result = (|| -> Result<Vec<u8>, String> {
        let encoded = env
            .convert_byte_array(&encoded)
            .map_err(|e| e.to_string())?;
        let image = if container == RAW_VVC {
            decode_266(&encoded)
        } else {
            decode(&encoded)
        }
        .map_err(|e| format!("VVC decode failed: {e}"))?;
        Ok(decoded_packet(&image))
    })();

    match result.and_then(|bytes| to_java_bytes(&mut env, &bytes)) {
        Ok(array) => array,
        Err(error) => {
            throw(&mut env, &error);
            std::ptr::null_mut()
        }
    }
}
