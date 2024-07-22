//
// Created by malik on 17.07.2024.
//

#include <jni.h>

#include <android/bitmap.h>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <cmath>
#include <string>
#include "ColorUtils.h"
#include "BitmapUtils.h"
#include "cairo.h"

extern "C" JNIEXPORT void JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_noiseImpl(
        JNIEnv *jenv, jobject clazz,
        jobject src, int threshold
) {
    srand(time(NULL));
    unsigned char *srcByteBuffer;
    int result = 0;
    int i, j;
    AndroidBitmapInfo srcInfo;

    result = AndroidBitmap_getInfo(jenv, src, &srcInfo);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        return;
    }

    result = AndroidBitmap_lockPixels(jenv, src, (void **) &srcByteBuffer);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        return;
    }

    int width = srcInfo.width;
    int height = srcInfo.height;

    for (int y = 0; y < height; ++y) {
        auto pixels = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(srcByteBuffer) +
                                                  y * srcInfo.stride);
        int x = 0;

        for (; x < width; ++x) {
            pixels[0] = pixels[0] | rand() % threshold;
            pixels[1] = pixels[1] | rand() % threshold;
            pixels[2] = pixels[2] | rand() % threshold;

            pixels += 4;
        }
    }
    AndroidBitmap_unlockPixels(jenv, src);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_shuffleBlurImpl(
        JNIEnv *jenv,
        jobject clazz,
        jobject src,
        jfloat threshold,
        jfloat strength
) {
    srand(std::chrono::system_clock::now().time_since_epoch().count());
    unsigned char *srcByteBuffer;
    int result;
    AndroidBitmapInfo srcInfo;

    result = AndroidBitmap_getInfo(jenv, src, &srcInfo);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) return;

    result = AndroidBitmap_lockPixels(jenv, src, (void **) &srcByteBuffer);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) return;

    int width = srcInfo.width;
    int height = srcInfo.height;

    for (int y = 0; y < height; ++y) {
        auto pixels = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(srcByteBuffer) +
                                                  y * srcInfo.stride);

        for (int x = 0; x < width; ++x) {
            float luma = luminance(pixels[0], pixels[1], pixels[2]);

            bool overflows = (threshold >= 0) ? (luma <= threshold) : (luma > abs(threshold));

            if (overflows) {
                int startY = y - (rand() % (y + 1)) * strength;
                int endY = y + (rand() % (y + 1)) * strength;
                int startX = x - (rand() % (x + 1)) * strength;
                int endX = x + (rand() % (x + 1)) * strength;

                int ranY = startY + (rand() % (endY - startY + 1));
                int ranX = startX + (rand() % (endX - startX + 1));

                int newX = std::clamp(ranX, 0, width - 1);
                int newY = std::clamp(ranY, 0, height - 1);
                auto newPixels = reinterpret_cast<uint8_t *>(
                        reinterpret_cast<uint8_t *>(srcByteBuffer) +
                        newY * srcInfo.stride);
                newPixels += 4 * newX;

                int newR = newPixels[0];
                int newG = newPixels[1];
                int newB = newPixels[2];

                pixels[0] = std::clamp(newR, 0, 255);
                pixels[1] = std::clamp(newG, 0, 255);
                pixels[2] = std::clamp(newB, 0, 255);
            }

            pixels += 4;
        }
    }
    AndroidBitmap_unlockPixels(jenv, src);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_cropToContentImpl(
        JNIEnv *env,
        jobject /* this */,
        jobject bitmap,
        jint colorToIgnore,
        jfloat tolerance
) {
    AndroidBitmapInfo info;
    void *pixels;
    int result;

    result = AndroidBitmap_getInfo(env, bitmap, &info);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    result = AndroidBitmap_lockPixels(env, bitmap, &pixels);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    int left = info.width, top = info.height, right = 0, bottom = 0;
    uint32_t *line = (uint32_t *) pixels;

    for (int y = 0; y < info.height; y++) {
        for (int x = 0; x < info.width; x++) {
            uint32_t pixel = line[x];

            if (colorDiff(pixel, colorToIgnore) / 255.0 <= tolerance) continue;

            if (x < left) left = x;
            if (x > right) right = x;
            if (y < top) top = y;
            if (y > bottom) bottom = y;
        }
        line = (uint32_t *) ((char *) line + info.stride);
    }

    std::vector<uint8_t> transient(info.stride * info.height);
    std::copy((uint8_t *) pixels, (uint8_t *) pixels + info.stride * info.height,
              transient.begin());

    AndroidBitmap_unlockPixels(env, bitmap);

    if (left > right || top > bottom) return nullptr;

    jobject newBitmap = createBitmap(env, right - left + 1, bottom - top + 1);

    if (!newBitmap) return nullptr;

    AndroidBitmapInfo newInfo;
    void *newPixels;

    result = AndroidBitmap_getInfo(env, newBitmap, &newInfo);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    result = AndroidBitmap_lockPixels(env, newBitmap, &newPixels);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;


    for (int y = top, j = 0; y <= bottom; y++, ++j) {
        memcpy((uint8_t *) newPixels + j * newInfo.stride,
               (uint8_t *) transient.data() + y * info.stride + left * 4, (right - left) * 4);
    }

    AndroidBitmap_unlockPixels(env, newBitmap);

    return newBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_drawColorImpl(JNIEnv *env, jobject thiz,
                                                                  jobject input, jint color) {
    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, input, &info) < 0) {
        return nullptr;
    }

    void *pixels;
    if (AndroidBitmap_lockPixels(env, input, &pixels) < 0) {
        return nullptr;
    }

    int width = (int) info.width;
    int height = (int) info.height;
    int stride = (int) info.stride;

    jobject newImage = createBitmap(env, width, height);
    if (newImage == nullptr) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    void *newPixels;
    if (AndroidBitmap_lockPixels(env, newImage, &newPixels) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    memcpy(newPixels, pixels, stride * height);

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
            reinterpret_cast<unsigned char *>(newPixels),
            CAIRO_FORMAT_ARGB32,
            width,
            height,
            stride
    );

    cairo_t *cr = cairo_create(surface);


    cairo_scale(cr, width, height);

    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

    ARGB argb = ColorToARGB(color);

    cairo_set_source_rgba(cr, argb.b / 255.0, argb.g / 255.0, argb.r / 255.0, argb.a / 255.0);
    cairo_rectangle(cr, 0.0, 0.0, 1.0, 1.0);
    cairo_fill(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, newImage);

    return newImage;
}