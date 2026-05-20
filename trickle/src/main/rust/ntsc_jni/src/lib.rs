#[link(name = "jnigraphics")]
unsafe extern "C" {}

use jni::JNIEnv;
use jni::objects::{JObject, JValue};
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
    random_seed: jint,
    use_field: jint,
    filter_type: jint,
    input_luma_filter: jint,
    chroma_lowpass_in: jint,
    chroma_demodulation: jint,
    luma_smear: jfloat,
    composite_sharpening: jfloat,
    video_scanline_phase_shift: jint,
    video_scanline_phase_shift_offset: jint,
    head_switching_enabled: jboolean,
    head_switching_height: jint,
    head_switching_offset: jint,
    head_switching_horizontal_shift: jfloat,
    head_switching_mid_line_enabled: jboolean,
    head_switching_mid_line_position: jfloat,
    head_switching_mid_line_jitter: jfloat,
    tracking_noise_enabled: jboolean,
    tracking_noise_height: jint,
    tracking_noise_wave_intensity: jfloat,
    tracking_noise_snow_intensity: jfloat,
    tracking_noise_snow_anisotropy: jfloat,
    tracking_noise_noise_intensity: jfloat,
    composite_noise_enabled: jboolean,
    composite_noise_frequency: jfloat,
    composite_noise_intensity: jfloat,
    composite_noise_detail: jint,
    ringing_enabled: jboolean,
    ringing_frequency: jfloat,
    ringing_power: jfloat,
    ringing_intensity: jfloat,
    luma_noise_enabled: jboolean,
    luma_noise_frequency: jfloat,
    luma_noise_intensity: jfloat,
    luma_noise_detail: jint,
    chroma_noise_enabled: jboolean,
    chroma_noise_frequency: jfloat,
    chroma_noise_intensity: jfloat,
    chroma_noise_detail: jint,
    snow_intensity: jfloat,
    snow_anisotropy: jfloat,
    chroma_phase_noise_intensity: jfloat,
    chroma_phase_error: jfloat,
    chroma_delay_horizontal: jfloat,
    chroma_delay_vertical: jint,
    vhs_enabled: jboolean,
    vhs_tape_speed: jint,
    vhs_chroma_loss: jfloat,
    vhs_sharpen_enabled: jboolean,
    vhs_sharpen_intensity: jfloat,
    vhs_sharpen_frequency: jfloat,
    vhs_edge_wave_enabled: jboolean,
    vhs_edge_wave_intensity: jfloat,
    vhs_edge_wave_speed: jfloat,
    vhs_edge_wave_frequency: jfloat,
    vhs_edge_wave_detail: jint,
    chroma_vert_blend: jboolean,
    chroma_lowpass_out: jint,
    scale_enabled: jboolean,
    scale_horizontal: jfloat,
    scale_vertical: jfloat,
    scale_with_video_size: jboolean,
) -> jobject {
    let mut buffer = match unsafe { read_bitmap(&mut env, &bitmap) } {
        Ok(buffer) => buffer,
        Err(error) => {
            throw(&mut env, &error);
            return std::ptr::null_mut();
        }
    };

    let mut effect = NtscEffect::default();
    effect.random_seed = random_seed;
    effect.use_field = enum_use_field(use_field);
    effect.filter_type = enum_filter_type(filter_type);
    effect.input_luma_filter = enum_luma_lowpass(input_luma_filter);
    effect.chroma_lowpass_in = enum_chroma_lowpass(chroma_lowpass_in);
    effect.chroma_demodulation = enum_chroma_demodulation(chroma_demodulation);
    effect.luma_smear = finite(luma_smear, 0.5);
    effect.composite_sharpening = finite(composite_sharpening, 1.0);
    effect.video_scanline_phase_shift = enum_phase_shift(video_scanline_phase_shift);
    effect.video_scanline_phase_shift_offset = video_scanline_phase_shift_offset;
    effect.head_switching = if jbool(head_switching_enabled) {
        Some(HeadSwitchingSettings {
            height: head_switching_height,
            offset: head_switching_offset,
            horiz_shift: finite(head_switching_horizontal_shift, 72.0),
            mid_line: if jbool(head_switching_mid_line_enabled) {
                Some(HeadSwitchingMidLineSettings {
                    position: finite(head_switching_mid_line_position, 0.95),
                    jitter: finite(head_switching_mid_line_jitter, 0.03),
                })
            } else {
                None
            },
        })
    } else {
        None
    };
    effect.tracking_noise = if jbool(tracking_noise_enabled) {
        Some(TrackingNoiseSettings {
            height: tracking_noise_height,
            wave_intensity: finite(tracking_noise_wave_intensity, 15.0),
            snow_intensity: finite(tracking_noise_snow_intensity, 0.025).max(0.0),
            snow_anisotropy: finite(tracking_noise_snow_anisotropy, 0.25),
            noise_intensity: finite(tracking_noise_noise_intensity, 0.25).max(0.0),
        })
    } else {
        None
    };
    effect.composite_noise = if jbool(composite_noise_enabled) {
        Some(FbmNoiseSettings {
            frequency: finite(composite_noise_frequency, 0.5),
            intensity: finite(composite_noise_intensity, 0.05),
            detail: composite_noise_detail,
        })
    } else {
        None
    };
    effect.ringing = if jbool(ringing_enabled) {
        Some(RingingSettings {
            frequency: finite(ringing_frequency, 0.45),
            power: finite(ringing_power, 4.0),
            intensity: finite(ringing_intensity, 4.0),
        })
    } else {
        None
    };
    effect.luma_noise = if jbool(luma_noise_enabled) {
        Some(FbmNoiseSettings {
            frequency: finite(luma_noise_frequency, 0.5),
            intensity: finite(luma_noise_intensity, 0.01),
            detail: luma_noise_detail,
        })
    } else {
        None
    };
    effect.chroma_noise = if jbool(chroma_noise_enabled) {
        Some(FbmNoiseSettings {
            frequency: finite(chroma_noise_frequency, 0.05),
            intensity: finite(chroma_noise_intensity, 0.1),
            detail: chroma_noise_detail,
        })
    } else {
        None
    };
    effect.snow_intensity = finite(snow_intensity, 0.00025).max(0.0);
    effect.snow_anisotropy = finite(snow_anisotropy, 0.5);
    effect.chroma_phase_noise_intensity = finite(chroma_phase_noise_intensity, 0.001).max(0.0);
    effect.chroma_phase_error = finite(chroma_phase_error, 0.0);
    effect.chroma_delay_horizontal = finite(chroma_delay_horizontal, 0.0);
    effect.chroma_delay_vertical = chroma_delay_vertical;
    effect.vhs_settings = if jbool(vhs_enabled) {
        Some(VHSSettings {
            tape_speed: enum_vhs_tape_speed(vhs_tape_speed),
            chroma_loss: finite(vhs_chroma_loss, 0.000025).clamp(0.0, 1.0),
            sharpen: if jbool(vhs_sharpen_enabled) {
                Some(VHSSharpenSettings {
                    intensity: finite(vhs_sharpen_intensity, 0.25),
                    frequency: finite(vhs_sharpen_frequency, 1.0),
                })
            } else {
                None
            },
            edge_wave: if jbool(vhs_edge_wave_enabled) {
                Some(VHSEdgeWaveSettings {
                    intensity: finite(vhs_edge_wave_intensity, 0.5),
                    speed: finite(vhs_edge_wave_speed, 4.0),
                    frequency: finite(vhs_edge_wave_frequency, 0.05),
                    detail: vhs_edge_wave_detail,
                })
            } else {
                None
            },
        })
    } else {
        None
    };
    effect.chroma_vert_blend = jbool(chroma_vert_blend);
    effect.chroma_lowpass_out = enum_chroma_lowpass(chroma_lowpass_out);
    effect.scale = if jbool(scale_enabled) {
        Some(ScaleSettings {
            horizontal_scale: positive_scale(scale_horizontal),
            vertical_scale: positive_scale(scale_vertical),
            scale_with_video_size: jbool(scale_with_video_size),
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
