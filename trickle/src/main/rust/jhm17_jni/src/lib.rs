#[path = "../../interruption.rs"]
mod interruption;
mod options;

use interruption::Interruption;

use jhm17::{ColorType, Encoder, SamplingFactor, rgb_to_ycbcr};
use jni::JNIEnv;
use jni::objects::{JByteArray, JFloatArray, JIntArray, JObject, JObjectArray, JValue};
use jni::sys::{JNI_TRUE, jboolean, jbyteArray, jint};
use std::sync::Arc;

fn read_bitmap(
    env: &mut JNIEnv,
    bitmap: &JObject,
    width: jint,
    height: jint,
    grayscale: bool,
    interruption: &Interruption,
) -> Result<Vec<u8>, String> {
    let channels = if grayscale { 1 } else { 3 };
    let len = (width as usize)
        .checked_mul(height as usize)
        .and_then(|pixels| pixels.checked_mul(channels))
        .ok_or("Bitmap is too large")?;
    let mut pixels = Vec::new();
    pixels.try_reserve_exact(len).map_err(|e| e.to_string())?;
    let row = env.new_int_array(width).map_err(|e| e.to_string())?;
    let mut argb = vec![0; width as usize];

    // Bitmap.getPixels returns straight sRGB colors, including for premultiplied bitmaps.
    for y in 0..height {
        interruption.check()?;
        env.call_method(
            bitmap,
            "getPixels",
            "([IIIIIII)V",
            &[
                JValue::Object(row.as_ref()),
                JValue::Int(0),
                JValue::Int(width),
                JValue::Int(0),
                JValue::Int(y),
                JValue::Int(width),
                JValue::Int(1),
            ],
        )
        .map_err(|e| e.to_string())?;
        env.get_int_array_region(&row, 0, &mut argb)
            .map_err(|e| e.to_string())?;
        for pixel in &argb {
            let [_, r, g, b] = pixel.to_be_bytes();
            if grayscale {
                pixels.push(rgb_to_ycbcr(r, g, b).0);
            } else {
                pixels.extend_from_slice(&[r, g, b]);
            }
        }
    }
    Ok(pixels)
}

#[unsafe(no_mangle)]
pub extern "system" fn Java_com_t8rin_trickle_Jhm17_encodeNative(
    mut env: JNIEnv,
    _this: JObject,
    bitmap: JObject,
    width: jint,
    height: jint,
    quality: jint,
    progressive: jboolean,
    chroma: jint,
    polish: jint,
    grayscale: jboolean,
    settings: JIntArray,
    rdo: JFloatArray,
    luma_table: JIntArray,
    chroma_table: JIntArray,
    luma_tuning: JFloatArray,
    cb_tuning: JFloatArray,
    cr_tuning: JFloatArray,
    deadzone: JFloatArray,
    custom_polish: JIntArray,
    icc: JByteArray,
    exif: JByteArray,
    app_numbers: JIntArray,
    app_data: JObjectArray,
) -> jbyteArray {
    let interruption = match Interruption::new(&mut env) {
        Ok(value) => Arc::new(value),
        Err(error) => {
            if !env.exception_check().unwrap_or(true) {
                let _ = env.throw_new("java/lang/RuntimeException", error.to_string());
            }
            return std::ptr::null_mut();
        }
    };
    let result = (|| {
        interruption.check()?;
        let mut output = Vec::new();
        let mut encoder = Encoder::new(&mut output, quality as u8);
        let check = Arc::clone(&interruption);
        encoder.set_cancel_check(move || check.is_requested());
        encoder.set_progressive(progressive == JNI_TRUE);
        let sampling = match chroma {
            1 => Some(SamplingFactor::R_4_2_0),
            2 => Some(SamplingFactor::R_4_2_2),
            3 => Some(SamplingFactor::R_4_4_4),
            4 => Some(SamplingFactor::R_4_4_0),
            5 => Some(SamplingFactor::R_4_4_1),
            6 => Some(SamplingFactor::R_4_2_1),
            7 => Some(SamplingFactor::R_4_1_1),
            8 => Some(SamplingFactor::R_4_1_0),
            _ => None,
        };
        if let Some(sampling) = sampling {
            encoder.set_sampling_factor(sampling);
        }
        options::apply_settings(&env, &mut encoder, &settings)?;
        options::apply_rdo(&env, &mut encoder, &rdo)?;
        options::apply_quantization(
            &env,
            &mut encoder,
            &luma_table,
            &chroma_table,
            &luma_tuning,
            &cb_tuning,
            &cr_tuning,
            &deadzone,
        )?;
        encoder.set_polish(options::read_polish(&env, polish, &custom_polish)?);
        options::apply_metadata(&mut env, &mut encoder, &icc, &exif, &app_numbers, &app_data)?;

        let grayscale = grayscale == JNI_TRUE;
        let pixels = read_bitmap(&mut env, &bitmap, width, height, grayscale, &interruption)?;
        let color = if grayscale {
            ColorType::Luma
        } else {
            ColorType::Rgb
        };
        encoder
            .encode(&pixels, width as u16, height as u16, color)
            .map_err(|e| e.to_string())?;
        interruption.check()?;
        env.byte_array_from_slice(&output)
            .map_err(|e| e.to_string())
    })();

    match result {
        Ok(array) => JByteArray::into_raw(array),
        Err(error) => {
            if matches!(env.exception_check(), Ok(false)) && !interruption.throw(&mut env) {
                let _ = env.throw_new("java/lang/RuntimeException", format!("jhm17: {error}"));
            }
            std::ptr::null_mut()
        }
    }
}
