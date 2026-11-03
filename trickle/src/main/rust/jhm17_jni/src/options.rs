use jhm17::{
    ChromaSubsamplingMethod, ClipOptions, DEFAULT_LUMA_BANDS, Deadzone, Encoder, JfifWrite,
    PixelDensity, PixelDensityUnit, Polish, PolishJudge, PolishOptions, ProgressiveMode,
    QuantizationTableType, RdoOptions, SamplePrecision, StepTuning,
};
use jni::JNIEnv;
use jni::objects::{JByteArray, JFloatArray, JIntArray, JObjectArray};

fn read_ints<const N: usize>(env: &JNIEnv, array: &JIntArray) -> Result<[i32; N], String> {
    let mut values = [0; N];
    env.get_int_array_region(array, 0, &mut values)
        .map_err(|e| e.to_string())?;
    Ok(values)
}

fn read_floats<const N: usize>(env: &JNIEnv, array: &JFloatArray) -> Result<[f32; N], String> {
    let mut values = [0.0; N];
    env.get_float_array_region(array, 0, &mut values)
        .map_err(|e| e.to_string())?;
    Ok(values)
}

pub(crate) fn apply_settings(
    env: &JNIEnv,
    encoder: &mut Encoder<impl JfifWrite>,
    settings: &JIntArray,
) -> Result<(), String> {
    if settings.is_null() {
        return Ok(());
    }
    let [
        progressive,
        scans,
        huffman,
        arithmetic,
        restart,
        subsampling,
        precision,
        chroma_scale,
        density_unit,
        density_x,
        density_y,
    ] = read_ints(env, settings)?;

    match progressive {
        0 => encoder.set_progressive_mode(None),
        1 => encoder.set_progressive_mode(Some(ProgressiveMode::Optimized)),
        2 => encoder.set_progressive_mode(Some(ProgressiveMode::Standard)),
        3 => encoder.set_progressive_scans(scans as u8),
        _ => {}
    }
    if huffman >= 0 {
        encoder.set_optimized_huffman_tables(huffman != 0);
    }
    if arithmetic >= 0 {
        encoder.set_arithmetic_coding(arithmetic != 0);
    }
    if restart >= 0 {
        encoder.set_restart_interval(restart as u16);
    }
    match subsampling {
        0 => encoder.set_chroma_subsampling_method(ChromaSubsamplingMethod::Nearest),
        1 => encoder.set_chroma_subsampling_method(ChromaSubsamplingMethod::Average),
        _ => {}
    }
    match precision {
        0 => encoder.set_sample_precision(SamplePrecision::Integer),
        1 => encoder.set_sample_precision(SamplePrecision::Luma),
        2 => encoder.set_sample_precision(SamplePrecision::All),
        _ => {}
    }
    if chroma_scale >= 0 {
        encoder.set_auto_chroma_scale(chroma_scale != 0);
    }
    if density_unit >= 0 {
        let unit = match density_unit {
            1 => PixelDensityUnit::Inches,
            2 => PixelDensityUnit::Centimeters,
            _ => PixelDensityUnit::PixelAspectRatio,
        };
        encoder.set_density(PixelDensity {
            density: (density_x as u16, density_y as u16),
            unit,
        });
    }
    Ok(())
}

pub(crate) fn apply_rdo(
    env: &JNIEnv,
    encoder: &mut Encoder<impl JfifWrite>,
    rdo: &JFloatArray,
) -> Result<(), String> {
    if rdo.is_null() {
        return Ok(());
    }
    let [
        lambda,
        chroma_weight,
        chroma_weight_full,
        distortion_power,
        tile_weights,
        tile_chroma,
        tile_chroma_full,
        aq_strength,
        aq_min,
        aq_max,
        aq_chroma,
        aq_brightness,
        aq_brightness_max,
        clip_iterations,
    ] = read_floats(env, rdo)?;
    encoder.set_rdo(Some(RdoOptions {
        lambda,
        chroma_weight,
        chroma_weight_full,
        distortion_power,
        tile_weights,
        tile_chroma: tile_chroma != 0.0,
        tile_chroma_full: tile_chroma_full != 0.0,
        aq_strength,
        aq_min,
        aq_max,
        aq_chroma: aq_chroma != 0.0,
        aq_brightness,
        aq_brightness_max,
        clip: ClipOptions {
            iterations: clip_iterations as u8,
        },
    }));
    Ok(())
}

fn read_table(env: &JNIEnv, table: &JIntArray) -> Result<QuantizationTableType, String> {
    use QuantizationTableType::*;

    if table.is_null() {
        return Ok(ImprovedDetectionModel);
    }
    if env.get_array_length(table).map_err(|e| e.to_string())? == 64 {
        let steps = read_ints::<64>(env, table)?;
        return Ok(Custom(Box::new(steps.map(|step| step as u16))));
    }
    let [preset] = read_ints(env, table)?;
    Ok(match preset {
        0 => Default,
        1 => Flat,
        2 => CustomMsSsim,
        3 => CustomPsnrHvs,
        4 => ImageMagick,
        5 => KleinSilversteinCarney,
        6 => DentalXRays,
        7 => VisualDetectionModel,
        _ => ImprovedDetectionModel,
    })
}

fn read_step_tuning(
    env: &JNIEnv,
    tuning: &JFloatArray,
    default: StepTuning,
) -> Result<StepTuning, String> {
    if tuning.is_null() {
        return Ok(default);
    }
    let [dc, ac, bands @ ..] = read_floats::<7>(env, tuning)?;
    Ok(StepTuning { dc, ac, bands })
}

pub(crate) fn apply_quantization(
    env: &JNIEnv,
    encoder: &mut Encoder<impl JfifWrite>,
    luma_table: &JIntArray,
    chroma_table: &JIntArray,
    luma_tuning: &JFloatArray,
    cb_tuning: &JFloatArray,
    cr_tuning: &JFloatArray,
    deadzone: &JFloatArray,
) -> Result<(), String> {
    if !luma_table.is_null() || !chroma_table.is_null() {
        encoder
            .set_quantization_tables(read_table(env, luma_table)?, read_table(env, chroma_table)?);
    }
    if !luma_tuning.is_null() || !cb_tuning.is_null() || !cr_tuning.is_null() {
        let luma = read_step_tuning(
            env,
            luma_tuning,
            StepTuning {
                bands: DEFAULT_LUMA_BANDS,
                ..StepTuning::default()
            },
        )?;
        let cb = read_step_tuning(env, cb_tuning, StepTuning::default())?;
        let cr = read_step_tuning(env, cr_tuning, StepTuning::default())?;
        encoder.set_step_tuning(luma, cb);
        encoder.set_chroma_step_tuning(cb, cr);
    }
    if !deadzone.is_null() {
        let [l0, l1, l2, l3, c0, c1, c2, c3, f0, f1, f2, f3] = read_floats(env, deadzone)?;
        encoder.set_deadzone(Deadzone {
            luma: [l0, l1, l2, l3],
            chroma: [c0, c1, c2, c3],
            chroma_full: [f0, f1, f2, f3],
        });
    }
    Ok(())
}

pub(crate) fn read_polish(env: &JNIEnv, polish: i32, custom: &JIntArray) -> Result<Polish, String> {
    if custom.is_null() {
        return Ok(match polish {
            1 => Polish::Greedy,
            2 => Polish::Trellis,
            3 => Polish::TrellisGreedy,
            _ => Polish::Off,
        });
    }
    let [
        judge,
        proposals,
        rounds,
        max_bits,
        restore_threshold,
        scan_limit,
        cvvdp_tries,
        quality_step,
        trellis_k,
        trellis_proposals,
    ] = read_ints(env, custom)?;
    let judge = match judge {
        0 => PolishJudge::Ssimulacra2,
        1 => PolishJudge::Cvvdp,
        _ => PolishJudge::Both,
    };
    Ok(Polish::Custom(PolishOptions {
        judge,
        proposals: proposals as usize,
        rounds: rounds as usize,
        max_bits,
        restore_threshold: f32::from_bits(restore_threshold as u32),
        scan_limit: scan_limit as usize,
        cvvdp_tries: cvvdp_tries as usize,
        quality_step: quality_step as u8,
        trellis_k: trellis_k as usize,
        trellis_proposals: trellis_proposals as usize,
    }))
}

pub(crate) fn apply_metadata(
    env: &mut JNIEnv,
    encoder: &mut Encoder<impl JfifWrite>,
    icc: &JByteArray,
    exif: &JByteArray,
    app_numbers: &JIntArray,
    app_data: &JObjectArray,
) -> Result<(), String> {
    if !icc.is_null() {
        let bytes = env.convert_byte_array(icc).map_err(|e| e.to_string())?;
        encoder.add_icc_profile(&bytes).map_err(|e| e.to_string())?;
    }
    if !exif.is_null() {
        let bytes = env.convert_byte_array(exif).map_err(|e| e.to_string())?;
        encoder
            .add_exif_metadata(&bytes)
            .map_err(|e| e.to_string())?;
    }
    if !app_numbers.is_null() {
        let count = env
            .get_array_length(app_numbers)
            .map_err(|e| e.to_string())?;
        for index in 0..count {
            let mut number = [0];
            env.get_int_array_region(app_numbers, index, &mut number)
                .map_err(|e| e.to_string())?;
            let number =
                u8::try_from(number[0]).map_err(|_| "JPEG APP segment number must be in 1..15")?;
            let data = env
                .get_object_array_element(app_data, index)
                .map_err(|e| e.to_string())?;
            let data = env.auto_local(JByteArray::from(data));
            let bytes = env.convert_byte_array(&data).map_err(|e| e.to_string())?;
            encoder
                .add_app_segment(number, bytes)
                .map_err(|e| e.to_string())?;
        }
    }
    Ok(())
}
