#[link(name = "jnigraphics")]
unsafe extern "C" {}

use jni::JNIEnv;
use jni::objects::{JBooleanArray, JFloatArray, JIntArray, JObject, JValue};
use jni::sys::{JNI_TRUE, jboolean, jfloat, jint, jobject};
use ntsc_rs::settings::standard::{
    ChromaDemodulationFilter, ChromaLowpass, FbmNoiseSettings, FilterType,
    HeadSwitchingMidLineSettings, HeadSwitchingSettings, LumaLowpass, NtscEffect, PhaseShift,
    RingingSettings, ScaleSettings, TrackingNoiseSettings, UseField, VHSEdgeWaveSettings,
    VHSSettings, VHSSharpenSettings, VHSTapeSpeed,
};
use ntsc_rs::yiq_fielding::Rgbx;

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
const INT_SETTINGS_LEN: usize = 18;
const FLOAT_SETTINGS_LEN: usize = 31;
const BOOLEAN_SETTINGS_LEN: usize = 13;

const I_RANDOM_SEED: usize = 0;
const I_USE_FIELD: usize = 1;
const I_FILTER_TYPE: usize = 2;
const I_INPUT_LUMA_FILTER: usize = 3;
const I_CHROMA_LOWPASS_IN: usize = 4;
const I_CHROMA_DEMODULATION: usize = 5;
const I_VIDEO_SCANLINE_PHASE_SHIFT: usize = 6;
const I_VIDEO_SCANLINE_PHASE_SHIFT_OFFSET: usize = 7;
const I_HEAD_SWITCHING_HEIGHT: usize = 8;
const I_HEAD_SWITCHING_OFFSET: usize = 9;
const I_TRACKING_NOISE_HEIGHT: usize = 10;
const I_COMPOSITE_NOISE_DETAIL: usize = 11;
const I_LUMA_NOISE_DETAIL: usize = 12;
const I_CHROMA_NOISE_DETAIL: usize = 13;
const I_CHROMA_DELAY_VERTICAL: usize = 14;
const I_VHS_TAPE_SPEED: usize = 15;
const I_VHS_EDGE_WAVE_DETAIL: usize = 16;
const I_CHROMA_LOWPASS_OUT: usize = 17;

const F_LUMA_SMEAR: usize = 0;
const F_COMPOSITE_SHARPENING: usize = 1;
const F_HEAD_SWITCHING_HORIZONTAL_SHIFT: usize = 2;
const F_HEAD_SWITCHING_MID_LINE_POSITION: usize = 3;
const F_HEAD_SWITCHING_MID_LINE_JITTER: usize = 4;
const F_TRACKING_NOISE_WAVE_INTENSITY: usize = 5;
const F_TRACKING_NOISE_SNOW_INTENSITY: usize = 6;
const F_TRACKING_NOISE_SNOW_ANISOTROPY: usize = 7;
const F_TRACKING_NOISE_NOISE_INTENSITY: usize = 8;
const F_COMPOSITE_NOISE_FREQUENCY: usize = 9;
const F_COMPOSITE_NOISE_INTENSITY: usize = 10;
const F_RINGING_FREQUENCY: usize = 11;
const F_RINGING_POWER: usize = 12;
const F_RINGING_INTENSITY: usize = 13;
const F_LUMA_NOISE_FREQUENCY: usize = 14;
const F_LUMA_NOISE_INTENSITY: usize = 15;
const F_CHROMA_NOISE_FREQUENCY: usize = 16;
const F_CHROMA_NOISE_INTENSITY: usize = 17;
const F_SNOW_INTENSITY: usize = 18;
const F_SNOW_ANISOTROPY: usize = 19;
const F_CHROMA_PHASE_NOISE_INTENSITY: usize = 20;
const F_CHROMA_PHASE_ERROR: usize = 21;
const F_CHROMA_DELAY_HORIZONTAL: usize = 22;
const F_VHS_CHROMA_LOSS: usize = 23;
const F_VHS_SHARPEN_INTENSITY: usize = 24;
const F_VHS_SHARPEN_FREQUENCY: usize = 25;
const F_VHS_EDGE_WAVE_INTENSITY: usize = 26;
const F_VHS_EDGE_WAVE_SPEED: usize = 27;
const F_VHS_EDGE_WAVE_FREQUENCY: usize = 28;
const F_SCALE_HORIZONTAL: usize = 29;
const F_SCALE_VERTICAL: usize = 30;

const B_HEAD_SWITCHING_ENABLED: usize = 0;
const B_HEAD_SWITCHING_MID_LINE_ENABLED: usize = 1;
const B_TRACKING_NOISE_ENABLED: usize = 2;
const B_COMPOSITE_NOISE_ENABLED: usize = 3;
const B_RINGING_ENABLED: usize = 4;
const B_LUMA_NOISE_ENABLED: usize = 5;
const B_CHROMA_NOISE_ENABLED: usize = 6;
const B_VHS_ENABLED: usize = 7;
const B_VHS_SHARPEN_ENABLED: usize = 8;
const B_VHS_EDGE_WAVE_ENABLED: usize = 9;
const B_CHROMA_VERT_BLEND: usize = 10;
const B_SCALE_ENABLED: usize = 11;
const B_SCALE_WITH_VIDEO_SIZE: usize = 12;

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

struct BitmapBuffer {
    width: u32,
    height: u32,
    pixels: Vec<u8>,
    alpha: Vec<u8>,
}

fn throw(env: &mut JNIEnv, msg: &str) {
    let _ = env.throw_new("java/lang/RuntimeException", msg);
}

fn jbool(value: jboolean) -> bool {
    value == JNI_TRUE
}

fn finite(value: jfloat, default: f32) -> f32 {
    if value.is_finite() { value } else { default }
}

fn positive_scale(value: jfloat) -> f32 {
    finite(value, 1.0).max(0.001)
}

fn read_int_settings(
    env: &mut JNIEnv,
    array: &JIntArray,
    expected_len: usize,
) -> Result<Vec<jint>, String> {
    let length = env.get_array_length(array).map_err(|e| e.to_string())? as usize;
    if length != expected_len {
        return Err(format!(
            "Invalid NTSC int settings length: expected {expected_len}, got {length}"
        ));
    }
    let mut values = vec![0; expected_len];
    env.get_int_array_region(array, 0, &mut values)
        .map_err(|e| e.to_string())?;
    Ok(values)
}

fn read_float_settings(
    env: &mut JNIEnv,
    array: &JFloatArray,
    expected_len: usize,
) -> Result<Vec<jfloat>, String> {
    let length = env.get_array_length(array).map_err(|e| e.to_string())? as usize;
    if length != expected_len {
        return Err(format!(
            "Invalid NTSC float settings length: expected {expected_len}, got {length}"
        ));
    }
    let mut values = vec![0.0; expected_len];
    env.get_float_array_region(array, 0, &mut values)
        .map_err(|e| e.to_string())?;
    Ok(values)
}

fn read_boolean_settings(
    env: &mut JNIEnv,
    array: &JBooleanArray,
    expected_len: usize,
) -> Result<Vec<jboolean>, String> {
    let length = env.get_array_length(array).map_err(|e| e.to_string())? as usize;
    if length != expected_len {
        return Err(format!(
            "Invalid NTSC boolean settings length: expected {expected_len}, got {length}"
        ));
    }
    let mut values = vec![0; expected_len];
    env.get_boolean_array_region(array, 0, &mut values)
        .map_err(|e| e.to_string())?;
    Ok(values)
}

fn enum_use_field(value: jint) -> UseField {
    match value {
        0 => UseField::Alternating,
        1 => UseField::Upper,
        2 => UseField::Lower,
        3 => UseField::Both,
        4 => UseField::InterleavedUpper,
        5 => UseField::InterleavedLower,
        _ => UseField::InterleavedUpper,
    }
}

fn enum_filter_type(value: jint) -> FilterType {
    match value {
        0 => FilterType::ConstantK,
        1 => FilterType::Butterworth,
        _ => FilterType::Butterworth,
    }
}

fn enum_luma_lowpass(value: jint) -> LumaLowpass {
    match value {
        0 => LumaLowpass::None,
        1 => LumaLowpass::Box,
        2 => LumaLowpass::Notch,
        _ => LumaLowpass::Notch,
    }
}

fn enum_phase_shift(value: jint) -> PhaseShift {
    match value {
        0 => PhaseShift::Degrees0,
        1 => PhaseShift::Degrees90,
        2 => PhaseShift::Degrees180,
        3 => PhaseShift::Degrees270,
        _ => PhaseShift::Degrees180,
    }
}

fn enum_vhs_tape_speed(value: jint) -> VHSTapeSpeed {
    match value {
        0 => VHSTapeSpeed::NONE,
        1 => VHSTapeSpeed::SP,
        2 => VHSTapeSpeed::LP,
        3 => VHSTapeSpeed::EP,
        _ => VHSTapeSpeed::LP,
    }
}

fn enum_chroma_lowpass(value: jint) -> ChromaLowpass {
    match value {
        0 => ChromaLowpass::None,
        1 => ChromaLowpass::Light,
        2 => ChromaLowpass::Full,
        _ => ChromaLowpass::Full,
    }
}

fn enum_chroma_demodulation(value: jint) -> ChromaDemodulationFilter {
    match value {
        0 => ChromaDemodulationFilter::Box,
        1 => ChromaDemodulationFilter::Notch,
        2 => ChromaDemodulationFilter::OneLineComb,
        3 => ChromaDemodulationFilter::TwoLineComb,
        _ => ChromaDemodulationFilter::Notch,
    }
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

    let width = info.width as usize;
    let height = info.height as usize;
    let row_bytes = width * 4;
    let stride = info.stride as usize;
    let src = pixels_ptr as *const u8;
    let mut pixels = vec![0u8; row_bytes * height];
    let mut alpha = vec![255u8; width * height];

    for y in 0..height {
        let src_row = unsafe { std::slice::from_raw_parts(src.add(y * stride), row_bytes) };
        let dst_offset = y * row_bytes;
        pixels[dst_offset..dst_offset + row_bytes].copy_from_slice(src_row);
        for x in 0..width {
            alpha[y * width + x] = src_row[x * 4 + 3];
        }
    }

    if unsafe { AndroidBitmap_unlockPixels(raw_env, raw_bitmap) } < 0 {
        return Err("AndroidBitmap_unlockPixels failed".to_string());
    }

    Ok(BitmapBuffer {
        width: info.width,
        height: info.height,
        pixels,
        alpha,
    })
}

fn create_bitmap<'local>(
    env: &mut JNIEnv<'local>,
    width: u32,
    height: u32,
) -> Result<JObject<'local>, String> {
    let bitmap_config = env
        .find_class("android/graphics/Bitmap$Config")
        .map_err(|e| e.to_string())?;
    let argb_8888 = env
        .get_static_field(
            bitmap_config,
            "ARGB_8888",
            "Landroid/graphics/Bitmap$Config;",
        )
        .map_err(|e| e.to_string())?
        .l()
        .map_err(|e| e.to_string())?;
    let bitmap_class = env
        .find_class("android/graphics/Bitmap")
        .map_err(|e| e.to_string())?;
    let args = [
        JValue::Int(width as jint),
        JValue::Int(height as jint),
        JValue::Object(&argb_8888),
    ];

    env.call_static_method(
        bitmap_class,
        "createBitmap",
        "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;",
        &args,
    )
    .map_err(|e| e.to_string())?
    .l()
    .map_err(|e| e.to_string())
}

unsafe fn write_bitmap(
    env: &mut JNIEnv,
    bitmap: &JObject,
    width: u32,
    height: u32,
    pixels: &[u8],
) -> Result<(), String> {
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
        return Err("AndroidBitmap_getInfo failed for output".to_string());
    }
    if info.format != ANDROID_BITMAP_FORMAT_RGBA_8888
        || info.width != width
        || info.height != height
    {
        return Err("Unexpected output bitmap layout".to_string());
    }

    let mut pixels_ptr: *mut std::ffi::c_void = std::ptr::null_mut();
    if unsafe { AndroidBitmap_lockPixels(raw_env, raw_bitmap, &mut pixels_ptr) } < 0 {
        return Err("AndroidBitmap_lockPixels failed for output".to_string());
    }

    let row_bytes = width as usize * 4;
    let stride = info.stride as usize;
    let dst = pixels_ptr as *mut u8;
    for y in 0..height as usize {
        let dst_row = unsafe { std::slice::from_raw_parts_mut(dst.add(y * stride), row_bytes) };
        let src_offset = y * row_bytes;
        dst_row.copy_from_slice(&pixels[src_offset..src_offset + row_bytes]);
    }

    if unsafe { AndroidBitmap_unlockPixels(raw_env, raw_bitmap) } < 0 {
        return Err("AndroidBitmap_unlockPixels failed for output".to_string());
    }

    Ok(())
}

fn restore_alpha(pixels: &mut [u8], alpha: &[u8]) {
    for (pixel, alpha) in pixels.chunks_exact_mut(4).zip(alpha.iter().copied()) {
        pixel[3] = alpha;
    }
}

#[allow(clippy::too_many_arguments)]
#[unsafe(no_mangle)]
pub unsafe extern "system" fn Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_ntscImpl(
    mut env: JNIEnv,
    _object: JObject,
    bitmap: JObject,
    frame: jint,
    scale_factor_x: jfloat,
    scale_factor_y: jfloat,
    int_settings: JIntArray,
    float_settings: JFloatArray,
    boolean_settings: JBooleanArray,
) -> jobject {
    let mut buffer = match unsafe { read_bitmap(&mut env, &bitmap) } {
        Ok(buffer) => buffer,
        Err(error) => {
            throw(&mut env, &error);
            return std::ptr::null_mut();
        }
    };
    let int_settings = match read_int_settings(&mut env, &int_settings, INT_SETTINGS_LEN) {
        Ok(settings) => settings,
        Err(error) => {
            throw(&mut env, &error);
            return std::ptr::null_mut();
        }
    };
    let float_settings = match read_float_settings(&mut env, &float_settings, FLOAT_SETTINGS_LEN) {
        Ok(settings) => settings,
        Err(error) => {
            throw(&mut env, &error);
            return std::ptr::null_mut();
        }
    };
    let boolean_settings =
        match read_boolean_settings(&mut env, &boolean_settings, BOOLEAN_SETTINGS_LEN) {
            Ok(settings) => settings,
            Err(error) => {
                throw(&mut env, &error);
                return std::ptr::null_mut();
            }
        };

    let mut effect = NtscEffect::default();
    effect.random_seed = int_settings[I_RANDOM_SEED];
    effect.use_field = enum_use_field(int_settings[I_USE_FIELD]);
    effect.filter_type = enum_filter_type(int_settings[I_FILTER_TYPE]);
    effect.input_luma_filter = enum_luma_lowpass(int_settings[I_INPUT_LUMA_FILTER]);
    effect.chroma_lowpass_in = enum_chroma_lowpass(int_settings[I_CHROMA_LOWPASS_IN]);
    effect.chroma_demodulation = enum_chroma_demodulation(int_settings[I_CHROMA_DEMODULATION]);
    effect.luma_smear = finite(float_settings[F_LUMA_SMEAR], 0.5);
    effect.composite_sharpening = finite(float_settings[F_COMPOSITE_SHARPENING], 1.0);
    effect.video_scanline_phase_shift =
        enum_phase_shift(int_settings[I_VIDEO_SCANLINE_PHASE_SHIFT]);
    effect.video_scanline_phase_shift_offset = int_settings[I_VIDEO_SCANLINE_PHASE_SHIFT_OFFSET];
    effect.head_switching = if jbool(boolean_settings[B_HEAD_SWITCHING_ENABLED]) {
        Some(HeadSwitchingSettings {
            height: int_settings[I_HEAD_SWITCHING_HEIGHT],
            offset: int_settings[I_HEAD_SWITCHING_OFFSET],
            horiz_shift: finite(float_settings[F_HEAD_SWITCHING_HORIZONTAL_SHIFT], 72.0),
            mid_line: if jbool(boolean_settings[B_HEAD_SWITCHING_MID_LINE_ENABLED]) {
                Some(HeadSwitchingMidLineSettings {
                    position: finite(float_settings[F_HEAD_SWITCHING_MID_LINE_POSITION], 0.95),
                    jitter: finite(float_settings[F_HEAD_SWITCHING_MID_LINE_JITTER], 0.03),
                })
            } else {
                None
            },
        })
    } else {
        None
    };
    effect.tracking_noise = if jbool(boolean_settings[B_TRACKING_NOISE_ENABLED]) {
        Some(TrackingNoiseSettings {
            height: int_settings[I_TRACKING_NOISE_HEIGHT],
            wave_intensity: finite(float_settings[F_TRACKING_NOISE_WAVE_INTENSITY], 15.0),
            snow_intensity: finite(float_settings[F_TRACKING_NOISE_SNOW_INTENSITY], 0.025).max(0.0),
            snow_anisotropy: finite(float_settings[F_TRACKING_NOISE_SNOW_ANISOTROPY], 0.25),
            noise_intensity: finite(float_settings[F_TRACKING_NOISE_NOISE_INTENSITY], 0.25)
                .max(0.0),
        })
    } else {
        None
    };
    effect.composite_noise = if jbool(boolean_settings[B_COMPOSITE_NOISE_ENABLED]) {
        Some(FbmNoiseSettings {
            frequency: finite(float_settings[F_COMPOSITE_NOISE_FREQUENCY], 0.5),
            intensity: finite(float_settings[F_COMPOSITE_NOISE_INTENSITY], 0.05),
            detail: int_settings[I_COMPOSITE_NOISE_DETAIL],
        })
    } else {
        None
    };
    effect.ringing = if jbool(boolean_settings[B_RINGING_ENABLED]) {
        Some(RingingSettings {
            frequency: finite(float_settings[F_RINGING_FREQUENCY], 0.45),
            power: finite(float_settings[F_RINGING_POWER], 4.0),
            intensity: finite(float_settings[F_RINGING_INTENSITY], 4.0),
        })
    } else {
        None
    };
    effect.luma_noise = if jbool(boolean_settings[B_LUMA_NOISE_ENABLED]) {
        Some(FbmNoiseSettings {
            frequency: finite(float_settings[F_LUMA_NOISE_FREQUENCY], 0.5),
            intensity: finite(float_settings[F_LUMA_NOISE_INTENSITY], 0.01),
            detail: int_settings[I_LUMA_NOISE_DETAIL],
        })
    } else {
        None
    };
    effect.chroma_noise = if jbool(boolean_settings[B_CHROMA_NOISE_ENABLED]) {
        Some(FbmNoiseSettings {
            frequency: finite(float_settings[F_CHROMA_NOISE_FREQUENCY], 0.05),
            intensity: finite(float_settings[F_CHROMA_NOISE_INTENSITY], 0.1),
            detail: int_settings[I_CHROMA_NOISE_DETAIL],
        })
    } else {
        None
    };
    effect.snow_intensity = finite(float_settings[F_SNOW_INTENSITY], 0.00025).max(0.0);
    effect.snow_anisotropy = finite(float_settings[F_SNOW_ANISOTROPY], 0.5);
    effect.chroma_phase_noise_intensity =
        finite(float_settings[F_CHROMA_PHASE_NOISE_INTENSITY], 0.001).max(0.0);
    effect.chroma_phase_error = finite(float_settings[F_CHROMA_PHASE_ERROR], 0.0);
    effect.chroma_delay_horizontal = finite(float_settings[F_CHROMA_DELAY_HORIZONTAL], 0.0);
    effect.chroma_delay_vertical = int_settings[I_CHROMA_DELAY_VERTICAL];
    effect.vhs_settings = if jbool(boolean_settings[B_VHS_ENABLED]) {
        Some(VHSSettings {
            tape_speed: enum_vhs_tape_speed(int_settings[I_VHS_TAPE_SPEED]),
            chroma_loss: finite(float_settings[F_VHS_CHROMA_LOSS], 0.000025).clamp(0.0, 1.0),
            sharpen: if jbool(boolean_settings[B_VHS_SHARPEN_ENABLED]) {
                Some(VHSSharpenSettings {
                    intensity: finite(float_settings[F_VHS_SHARPEN_INTENSITY], 0.25),
                    frequency: finite(float_settings[F_VHS_SHARPEN_FREQUENCY], 1.0),
                })
            } else {
                None
            },
            edge_wave: if jbool(boolean_settings[B_VHS_EDGE_WAVE_ENABLED]) {
                Some(VHSEdgeWaveSettings {
                    intensity: finite(float_settings[F_VHS_EDGE_WAVE_INTENSITY], 0.5),
                    speed: finite(float_settings[F_VHS_EDGE_WAVE_SPEED], 4.0),
                    frequency: finite(float_settings[F_VHS_EDGE_WAVE_FREQUENCY], 0.05),
                    detail: int_settings[I_VHS_EDGE_WAVE_DETAIL],
                })
            } else {
                None
            },
        })
    } else {
        None
    };
    effect.chroma_vert_blend = jbool(boolean_settings[B_CHROMA_VERT_BLEND]);
    effect.chroma_lowpass_out = enum_chroma_lowpass(int_settings[I_CHROMA_LOWPASS_OUT]);
    effect.scale = if jbool(boolean_settings[B_SCALE_ENABLED]) {
        Some(ScaleSettings {
            horizontal_scale: positive_scale(float_settings[F_SCALE_HORIZONTAL]),
            vertical_scale: positive_scale(float_settings[F_SCALE_VERTICAL]),
            scale_with_video_size: jbool(boolean_settings[B_SCALE_WITH_VIDEO_SIZE]),
        })
    } else {
        None
    };

    let frame_num = frame.max(0) as usize;
    let dimensions = (buffer.width as usize, buffer.height as usize);
    let scale_factor = [
        positive_scale(scale_factor_x),
        positive_scale(scale_factor_y),
    ];
    let process_result = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
        effect.apply_effect_to_buffer::<Rgbx, u8>(
            dimensions,
            &mut buffer.pixels,
            frame_num,
            scale_factor,
        );
    }));
    if process_result.is_err() {
        throw(&mut env, "ntsc-rs processing failed");
        return std::ptr::null_mut();
    }
    restore_alpha(&mut buffer.pixels, &buffer.alpha);

    let output = match create_bitmap(&mut env, buffer.width, buffer.height) {
        Ok(bitmap) => bitmap,
        Err(error) => {
            throw(&mut env, &error);
            return std::ptr::null_mut();
        }
    };

    if let Err(error) = unsafe {
        write_bitmap(
            &mut env,
            &output,
            buffer.width,
            buffer.height,
            &buffer.pixels,
        )
    } {
        throw(&mut env, &error);
        return std::ptr::null_mut();
    }

    output.into_raw()
}
