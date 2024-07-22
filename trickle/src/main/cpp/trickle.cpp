#include <android/bitmap.h>
#include <__functional/function.h>
#include "Util.h"
#include "OilFilter.h"
#include "TvFilter.h"
#include "SketchFilter.h"
#include "AverageSmoothFilter.h"
#include "GaussianBlurFilter.h"
#include "HDRFilter.h"
#include "SharpenFilter.h"
#include "MotionBlurFilter.h"
#include "GothamFilter.h"
#include "ColorUtils.h"
#include "BitmapUtils.h"

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_oilImpl(JNIEnv *env,
                                                            jobject object, jobject bitmap,
                                                            jint oilRange) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    OilFilterOptions options(oilRange);

    int *result = PROC_IMAGE_WITH_OPTIONS(env, pixels, width, height, OilFilter, options);
    return createBitmap(env, result, width, height, stride, true);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_tvImpl(JNIEnv *env, jobject object,
                                                           jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    int *result = PROC_IMAGE_WITHOUT_OPTIONS(env, pixels, width, height, TvFilter);
    return createBitmap(env, result, width, height, stride, true);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_hdrImpl(JNIEnv *env, jobject object,
                                                            jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    int *result = PROC_IMAGE_WITHOUT_OPTIONS(env, pixels, width, height, HDRFilter);
    return createBitmap(env, result, width, height, stride, true);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_sketchImpl(JNIEnv *env, jobject object,
                                                               jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    int *result = PROC_IMAGE_WITHOUT_OPTIONS(env, pixels, width, height, SketchFilter);

    return createBitmap(env, result, width, height, stride, true);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_gothamImpl(JNIEnv *env, jobject object,
                                                               jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    int *result = PROC_IMAGE_WITHOUT_OPTIONS(env, pixels, width, height, GothamFilter);
    return createBitmap(env, result, width, height, stride, true);
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_colorPosterizeImpl(JNIEnv *env, jobject thiz,
                                                                       jobject input,
                                                                       jintArray colors) {
    AndroidBitmapInfo sourceInfo;
    void *sourcePixels;
    if (AndroidBitmap_getInfo(env, input, &sourceInfo) < 0) {
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, input, &sourcePixels) < 0) {
        return nullptr;
    }

    uint32_t width = sourceInfo.width;
    uint32_t height = sourceInfo.height;
    uint32_t stride = sourceInfo.stride;

    jsize len = (*env).GetArrayLength(colors);
    jint *body = (*env).GetIntArrayElements(colors, nullptr);
    RGB rgbColors[len];
    for (int i = 0; i < len; i++) {
        rgbColors[i] = ColorToRGB(body[i]);
    }

    jobject resultBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo resultInfo;
    void *resultPixels;
    if (AndroidBitmap_getInfo(env, resultBitmap, &resultInfo) < 0) {
        AndroidBitmap_unlockPixels(env, resultBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, resultBitmap, &resultPixels) < 0) {
        AndroidBitmap_unlockPixels(env, resultBitmap);
        return nullptr;
    }

    for (int y = 0; y < height; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);

        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(resultPixels) +
                                               y * stride);
        int x = 0;

        for (; x < width; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int srcAlpha = src[3];

            if (srcAlpha > 0) {
                double minDiff = 1000.;
                RGB rgb;

                for (auto paletteColor: rgbColors) {
                    double newDiff = colorDiff(paletteColor, RGB(r, g, b)) / 255.0;
                    if (newDiff < minDiff) {
                        minDiff = newDiff;
                        rgb = paletteColor;
                    }
                }

                dst[0] = rgb.r;
                dst[1] = rgb.g;
                dst[2] = rgb.b;
            }

            dst[3] = srcAlpha;

            dst += 4;
            src += 4;
        }
    }

    if (AndroidBitmap_unlockPixels(env, input) < 0) {
        AndroidBitmap_unlockPixels(env, resultBitmap);
        return nullptr;
    }

    if (AndroidBitmap_unlockPixels(env, resultBitmap) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    return resultBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_replaceColorImpl(JNIEnv *env, jobject thiz,
                                                                     jobject input,
                                                                     jint source_color,
                                                                     jint target_color,
                                                                     jfloat tolerance) {
    AndroidBitmapInfo sourceInfo;
    void *sourcePixels;
    if (AndroidBitmap_getInfo(env, input, &sourceInfo) < 0) {
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, input, &sourcePixels) < 0) {
        return nullptr;
    }

    uint32_t width = sourceInfo.width;
    uint32_t height = sourceInfo.height;
    uint32_t stride = sourceInfo.stride;

    RGB sourceColor = ColorToRGB(source_color);
    ARGB targetColor = ColorToARGB(target_color);

    jobject resultBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo resultInfo;
    void *resultPixels;
    if (AndroidBitmap_getInfo(env, resultBitmap, &resultInfo) < 0) {
        AndroidBitmap_unlockPixels(env, resultBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, resultBitmap, &resultPixels) < 0) {
        AndroidBitmap_unlockPixels(env, resultBitmap);
        return nullptr;
    }

    for (int y = 0; y < height; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);

        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(resultPixels) +
                                               y * stride);
        int x = 0;

        for (; x < width; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int srcAlpha = src[3];

            RGB pixel = RGB(r, g, b);

            ARGB argb;

            if (colorDiff(pixel, sourceColor) / 255.0 <= cbrt(3) * tolerance) {
                argb = targetColor;
            } else {
                argb = ARGB(srcAlpha, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    if (AndroidBitmap_unlockPixels(env, input) < 0) {
        AndroidBitmap_unlockPixels(env, resultBitmap);
        return nullptr;
    }

    if (AndroidBitmap_unlockPixels(env, resultBitmap) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    return resultBitmap;
}