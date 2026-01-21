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
#include "WarpEngine.cpp"

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_oilImpl(JNIEnv *env,
                                                            jobject object, jobject bitmap,
                                                            jint oilRange) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    OilFilterOptions options(oilRange);

    int *result = PROC_IMAGE_WITH_OPTIONS(env, pixels, width, height, OilFilter, options);

    AndroidBitmap_unlockPixels(env, bitmap);

    return createBitmap(env, result, width, height, stride);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_tvImpl(JNIEnv *env, jobject object,
                                                           jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    int *result = PROC_IMAGE_WITHOUT_OPTIONS(env, pixels, width, height, TvFilter);

    AndroidBitmap_unlockPixels(env, bitmap);

    return createBitmap(env, result, width, height, stride);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_hdrImpl(JNIEnv *env, jobject object,
                                                            jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    int *result = PROC_IMAGE_WITHOUT_OPTIONS(env, pixels, width, height, HDRFilter);

    AndroidBitmap_unlockPixels(env, bitmap);

    return createBitmap(env, result, width, height, stride);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_sketchImpl(JNIEnv *env, jobject object,
                                                               jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    int *result = PROC_IMAGE_WITHOUT_OPTIONS(env, pixels, width, height, SketchFilter);

    AndroidBitmap_unlockPixels(env, bitmap);

    return createBitmap(env, result, width, height, stride);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_gothamImpl(JNIEnv *env, jobject object,
                                                               jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    uint32_t width = info.width;
    uint32_t height = info.height;
    uint32_t stride = info.stride;

    int *result = PROC_IMAGE_WITHOUT_OPTIONS(env, pixels, width, height, GothamFilter);

    AndroidBitmap_unlockPixels(env, bitmap);

    return createBitmap(env, result, width, height, stride);
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

extern "C"
JNIEXPORT jlong JNICALL
Java_com_t8rin_trickle_WarpEngine_nativeCreate(
        JNIEnv* env,
        jobject,
        jobject bitmap
) {
    AndroidBitmapInfo info;
    void* ptr;

    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &ptr);

    auto* engine = new WarpEngine(info, ptr);

    AndroidBitmap_unlockPixels(env, bitmap);
    return reinterpret_cast<jlong>(engine);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_t8rin_trickle_WarpEngine_nativeApplyStroke(
        JNIEnv*,
        jobject,
        jlong handle,
        jfloat fromX,
        jfloat fromY,
        jfloat toX,
        jfloat toY,
        jfloat radius,
        jfloat hardness,
        jfloat strength,
        jint mode
) {
    auto* engine = reinterpret_cast<WarpEngine*>(handle);

    WarpBrush brush{
            radius,
            hardness,
            strength
    };

    engine->applyStroke(
            fromX,
            fromY,
            toX,
            toY,
            brush,
            (WarpMode)mode
    );
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_WarpEngine_nativeRender(
        JNIEnv* env,
        jobject,
        jlong handle
) {
    auto* engine = reinterpret_cast<WarpEngine*>(handle);

    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmap = env->GetStaticMethodID(
            bitmapCls,
            "createBitmap",
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;"
    );

    jclass configCls = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID argb8888 = env->GetStaticFieldID(
            configCls,
            "ARGB_8888",
            "Landroid/graphics/Bitmap$Config;"
    );

    jobject config = env->GetStaticObjectField(configCls, argb8888);

    jobject outBitmap = env->CallStaticObjectMethod(
            bitmapCls,
            createBitmap,
            engine->w,
            engine->h,
            config
    );

    void* outPtr;
    AndroidBitmap_lockPixels(env, outBitmap, &outPtr);
    engine->render(outPtr);
    AndroidBitmap_unlockPixels(env, outBitmap);

    return outBitmap;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_t8rin_trickle_WarpEngine_nativeDestroy(
        JNIEnv*,
        jobject,
        jlong handle
) {
    delete reinterpret_cast<WarpEngine*>(handle);
}