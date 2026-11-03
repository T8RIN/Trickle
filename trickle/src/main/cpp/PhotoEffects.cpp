#include <android/bitmap.h>
#include <limits>
#include <new>
#include "BitmapUtils.h"
#include "PhotoEffects.h"

namespace {

class LockedBitmap {
public:
    LockedBitmap(JNIEnv *env, jobject bitmap) : env(env), bitmap(bitmap) {
        if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != ANDROID_BITMAP_RESULT_SUCCESS) {
            pixels = nullptr;
        }
    }

    ~LockedBitmap() {
        if (pixels) AndroidBitmap_unlockPixels(env, bitmap);
    }

    LockedBitmap(const LockedBitmap &) = delete;
    LockedBitmap &operator=(const LockedBitmap &) = delete;

    uint8_t *data() const { return static_cast<uint8_t *>(pixels); }

private:
    JNIEnv *env;
    jobject bitmap;
    void *pixels = nullptr;
};

jobject applyPhotoFilter(
        JNIEnv *env, jobject input, photo::Filter filter, const photo::Params &params,
        uint32_t extra = 0, float strength = 1.f) {
    const EncodingInterruption interruption(env);
    if (interruption.throwIfRequested()) return nullptr;

    AndroidBitmapInfo sourceInfo{}, targetInfo{};
    if (AndroidBitmap_getInfo(env, input, &sourceInfo) != ANDROID_BITMAP_RESULT_SUCCESS ||
        sourceInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888 || sourceInfo.width == 0 ||
        sourceInfo.height == 0 || sourceInfo.width > std::numeric_limits<int>::max() ||
        sourceInfo.height > std::numeric_limits<int>::max()) return nullptr;

    auto result = createBitmap(env, sourceInfo.width, sourceInfo.height);
    if (!result || env->ExceptionCheck() ||
        AndroidBitmap_getInfo(env, result, &targetInfo) != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    try {
        const LockedBitmap source(env, input);
        if (!source.data()) return nullptr;
        const LockedBitmap target(env, result);
        if (!target.data()) return nullptr;
        const photo::Image image{int(sourceInfo.width), int(sourceInfo.height), sourceInfo.stride, source.data(),
                                 (sourceInfo.flags & ANDROID_BITMAP_FLAGS_ALPHA_MASK) == ANDROID_BITMAP_FLAGS_ALPHA_PREMUL};
        const photo::Output output{targetInfo.stride, target.data()};
        filter(image, output, params, extra, strength);
    } catch (const std::bad_alloc &) {
        if (!interruption.throwIfRequested()) {
            if (const auto error = env->FindClass("java/lang/OutOfMemoryError")) {
                env->ThrowNew(error, "Cannot allocate filter workspace");
            }
        }
        return nullptr;
    }

    return interruption.throwIfRequested() ? nullptr : result;
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_palCompositeImpl(
        JNIEnv *env, jobject, jobject input,
        jfloat lumaFilter, jint chromaRadius, jfloat noise, jint phaseError, jint scanSamples, jfloat
        lineCompensation, jint seed, jfloat strength) {
    return applyPhotoFilter(env, input, photo::palComposite,
                            {lumaFilter, float(chromaRadius), noise, float(phaseError), float(scanSamples),
                            lineCompensation}, uint32_t(seed), strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_secamImpl(
        JNIEnv *env, jobject, jobject input,
        jint chromaRadius, jfloat frequencyDeviation, jfloat noise, jfloat preEmphasis, jfloat lineCrosstalk, jint
        seed, jfloat strength) {
    return applyPhotoFilter(env, input, photo::secam,
                            {float(chromaRadius), frequencyDeviation, noise, preEmphasis, lineCrosstalk},
                            uint32_t(seed), strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_rfReceptionImpl(
        JNIEnv *env, jobject, jobject input,
        jint delay, jfloat reflection, jfloat noise, jint frequency, jint seed, jfloat strength) {
    return applyPhotoFilter(env, input, photo::rfReception,
                            {float(delay), reflection, noise, float(frequency)}, uint32_t(seed), strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_sstvReceptionImpl(
        JNIEnv *env, jobject, jobject input,
        jint transmissionMode, jfloat frequencyDrift, jfloat syncError, jfloat noise, jfloat dropout, jint seed,
        jfloat strength) {
    return applyPhotoFilter(env, input, photo::sstvReception,
                            {float(transmissionMode), frequencyDrift, syncError, noise, dropout}, uint32_t(seed),
                            strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_lcdSubpixelsImpl(
        JNIEnv *env, jobject, jobject input,
        jint size, jfloat blackMatrix, jint subpixelOrder, jint viewAngle, jfloat strength) {
    return applyPhotoFilter(env, input, photo::lcdSubpixels,
                            {float(size), blackMatrix, float(subpixelOrder), float(viewAngle)}, 0, strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_filmGrainImpl(
        JNIEnv *env, jobject, jobject input,
        jfloat size, jfloat intensity, jfloat sizeVariation, jfloat exposureDependence, jfloat colorGrain, jint
        seed) {
    return applyPhotoFilter(env, input, photo::filmGrain,
                            {size, intensity, sizeVariation, exposureDependence, colorGrain}, uint32_t(seed));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_filmNegativeImpl(
        JNIEnv *env, jobject, jobject input,
        jfloat exposure, jfloat densityRange, jfloat contrast, jfloat toneCurve, jint filmBase, jfloat strength) {
    return applyPhotoFilter(env, input, photo::filmNegative,
                            {exposure, densityRange, contrast, toneCurve}, uint32_t(filmBase), strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_agxToneMappingImpl(
        JNIEnv *env, jobject, jobject input,
        jfloat exposure, jint look, jfloat saturation, jfloat strength) {
    return applyPhotoFilter(env, input, photo::agxToneMapping,
                            {exposure, float(look), saturation}, 0, strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_waveletDenoiseImpl(
        JNIEnv *env, jobject, jobject input,
        jfloat lumaNoise, jfloat chromaNoise, jint levels, jfloat strength) {
    return applyPhotoFilter(env, input, photo::waveletDenoise,
                            {lumaNoise, chromaNoise, float(levels)}, 0, strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_daltonizationImpl(
        JNIEnv *env, jobject, jobject input,
        jint visionType, jfloat severity, jfloat correction, jfloat preserveLuminance, jfloat strength) {
    return applyPhotoFilter(env, input, photo::daltonization,
                            {float(visionType), severity, correction, preserveLuminance}, 0, strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_retroHardwareImpl(
        JNIEnv *env, jobject, jobject input,
        jint system, jfloat scale, jfloat dithering, jfloat strength) {
    return applyPhotoFilter(env, input, photo::retroHardware,
                            {float(system), scale, dithering}, 0, strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_risographImpl(
        JNIEnv *env, jobject, jobject input,
        jint spacing, jfloat misregistration, jfloat dotGain, jint inks, jfloat paperGrain, jint seed, jfloat
        strength) {
    return applyPhotoFilter(env, input, photo::risograph,
                            {float(spacing), misregistration, dotGain, float(inks), paperGrain}, uint32_t(seed),
                            strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_cmykPressImpl(
        JNIEnv *env, jobject, jobject input,
        jint spacing, jfloat blackGeneration, jfloat dotGain, jfloat misregistration, jfloat inkLimit, jfloat
        paperGrain, jint seed, jfloat strength) {
    return applyPhotoFilter(env, input, photo::cmykPress,
                            {float(spacing), blackGeneration, dotGain, misregistration, inkLimit, paperGrain},
                            uint32_t(seed), strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_thermalReceiptImpl(
        JNIEnv *env, jobject, jobject input,
        jint size, jfloat heat, jfloat heatRetention, jfloat defects, jfloat fading, jint seed, jfloat strength) {
    return applyPhotoFilter(env, input, photo::thermalReceipt,
                            {float(size), heat, heatRetention, defects, fading}, uint32_t(seed), strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_photoCrossStitchImpl(
        JNIEnv *env, jobject, jobject input,
        jint size, jint threadColors, jfloat threadWidth, jint lightAngle, jfloat relief, jint seed, jfloat
        strength) {
    return applyPhotoFilter(env, input, photo::crossStitch,
                            {float(size), float(threadColors), threadWidth, float(lightAngle), relief},
                            uint32_t(seed), strength);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_autostereogramImpl(
        JNIEnv *env, jobject, jobject input,
        jfloat repeatDistance, jfloat depth, jfloat gamma, jint size, jfloat softness, jint invert, jint seed) {
    return applyPhotoFilter(env, input, photo::autostereogram,
                            {repeatDistance, depth, gamma, float(size), softness, float(invert)}, uint32_t(seed));
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_paletteRemapImpl(
        JNIEnv *env, jobject, jobject input, jintArray palette, jfloat dithering) {
    const jsize count = env->GetArrayLength(palette);
    if (count < 1 || count > 7) return nullptr;

    std::array<jint, 7> colors{};
    env->GetIntArrayRegion(palette, 0, count, colors.data());
    if (env->ExceptionCheck()) return nullptr;

    photo::Params params{dithering};
    for (jsize i = 0; i < count; ++i) {
        params[i + 1] = float(colors[i] & 0xFFFFFF);
    }
    return applyPhotoFilter(env, input, photo::paletteRemap, params, uint32_t(count));
}
