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
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <vector>
#include "Eigen/Dense"
#include "unsupported/Eigen/CXX11/Tensor"

using namespace Eigen;

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
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_drawColorAboveImpl(JNIEnv *env, jobject thiz,
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

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_drawColorBehindImpl(JNIEnv *env, jobject thiz,
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

    memset(newPixels, 0, stride * height);

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
            reinterpret_cast<unsigned char *>(newPixels),
            CAIRO_FORMAT_ARGB32,
            width,
            height,
            stride
    );

    cairo_surface_t *image = cairo_image_surface_create_for_data(
            reinterpret_cast<unsigned char *>(pixels),
            CAIRO_FORMAT_ARGB32,
            width,
            height,
            stride
    );

    cairo_t *cr = cairo_create(surface);

    ARGB argb = ColorToARGB(color);
    cairo_set_source_rgba(cr, argb.b / 255.0, argb.g / 255.0, argb.r / 255.0, argb.a / 255.0);
    cairo_rectangle(cr, 0.0, 0.0, width, height);
    cairo_fill(cr);

    cairo_set_source_surface(cr, image, 0.0, 0.0);
    cairo_paint(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    cairo_surface_destroy(image);

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, newImage);

    return newImage;
}

ARGB mixColors(float t, uint32_t color1, uint32_t color2) {
    uint8_t a1 = (color1 >> 24) & 0xFF;
    uint8_t r1 = (color1 >> 16) & 0xFF;
    uint8_t g1 = (color1 >> 8) & 0xFF;
    uint8_t b1 = color1 & 0xFF;

    uint8_t a2 = (color2 >> 24) & 0xFF;
    uint8_t r2 = (color2 >> 16) & 0xFF;
    uint8_t g2 = (color2 >> 8) & 0xFF;
    uint8_t b2 = color2 & 0xFF;

    uint8_t a = static_cast<uint8_t>(a1 + t * (a2 - a1));
    uint8_t r = static_cast<uint8_t>(r1 + t * (r2 - r1));
    uint8_t g = static_cast<uint8_t>(g1 + t * (g2 - g1));
    uint8_t b = static_cast<uint8_t>(b1 + t * (b2 - b1));

    return {a, r, g, b};
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_tritoneImpl(JNIEnv *env, jobject thiz,
                                                                jobject input, jint shadows_color,
                                                                jint middle_color,
                                                                jint highlights_color) {
    AndroidBitmapInfo info;
    void *pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, input, &info)) < 0) {
        return nullptr;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return nullptr;
    }

    if ((ret = AndroidBitmap_lockPixels(env, input, &pixels)) < 0) {
        return nullptr;
    }

    int width = info.width;
    int height = info.height;
    int stride = info.stride;

    jobject outBitmap = createBitmap(env, width, height);
    void *outPixels;
    if ((ret = AndroidBitmap_lockPixels(env, outBitmap, &outPixels)) < 0) {
        return nullptr;
    }

    ARGB lut[256];
    for (int i = 0; i < 128; ++i) {
        float t = i / 127.0f;
        lut[i] = mixColors(t, shadows_color, middle_color);
    }
    for (int i = 128; i < 256; ++i) {
        float t = (i - 127) / 128.0f;
        lut[i] = mixColors(t, middle_color, highlights_color);
    }

    for (uint32_t y = 0; y < height; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(pixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outPixels) +
                                               y * stride);

        for (uint32_t x = 0; x < width; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            auto gray = static_cast<uint8_t>((0.299f * r + 0.587f * g + 0.114f * b));

            ARGB argb = lut[gray];

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outBitmap);

    return outBitmap;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_polkaDotImpl(JNIEnv *env, jobject obj,
                                                                 jobject bitmap, jint dotRadius,
                                                                 jint spacing) {
    AndroidBitmapInfo info;
    void *pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) != 0) {
        return nullptr;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return nullptr;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) != 0) {
        return nullptr;
    }

    jobject newBitmap = createBitmap(env, info.width, info.height);

    void *newPixels;
    if ((ret = AndroidBitmap_lockPixels(env, newBitmap, &newPixels)) != 0) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    int *srcPixels = (int *) pixels;
    int *dstPixels = (int *) newPixels;
    int width = info.width;
    int height = info.height;

    int dotDiameter = dotRadius * 2;
    int spacingBetweenDots = dotDiameter + spacing;
    int centerOffset = dotRadius;

    for (int y = 0; y < height; y += spacingBetweenDots) {
        for (int x = 0; x < width; x += spacingBetweenDots) {
            int centerX = x + centerOffset;
            int centerY = y + centerOffset;

            if (centerX >= width || centerY >= height) continue;

            int centerIndex = centerY * width + centerX;
            int centerColor = srcPixels[centerIndex];

            for (int dy = -dotRadius; dy <= dotRadius; dy++) {
                for (int dx = -dotRadius; dx <= dotRadius; dx++) {
                    int currentX = centerX + dx;
                    int currentY = centerY + dy;

                    if (currentX >= 0 && currentX < width && currentY >= 0 && currentY < height) {
                        int distanceSquared = dx * dx + dy * dy;
                        if (distanceSquared <= dotRadius * dotRadius) {
                            int currentIndex = currentY * width + currentX;
                            dstPixels[currentIndex] = centerColor;
                        }
                    }
                }
            }
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);
    AndroidBitmap_unlockPixels(env, newBitmap);

    return newBitmap;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_applyLutImpl(
        JNIEnv *env, jobject thiz, jobject input, jobject lutBitmap, jfloat intensity) {

    AndroidBitmapInfo srcInfo;
    void *srcPixels;
    if (AndroidBitmap_getInfo(env, input, &srcInfo) < 0) {
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, input, &srcPixels) < 0) {
        return nullptr;
    }

    AndroidBitmapInfo lutInfo;
    void *lutPixels;
    if (AndroidBitmap_getInfo(env, lutBitmap, &lutInfo) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, lutBitmap, &lutPixels) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    uint32_t srcWidth = srcInfo.width;
    uint32_t srcHeight = srcInfo.height;
    uint32_t lutWidth = lutInfo.width;

    jobject outputBitmap = createBitmap(env, srcWidth, srcHeight);
    if (outputBitmap == nullptr) {
        AndroidBitmap_unlockPixels(env, input);
        AndroidBitmap_unlockPixels(env, lutBitmap);
        return nullptr;
    }

    void *outputPixels;
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        AndroidBitmap_unlockPixels(env, lutBitmap);
        return nullptr;
    }

    for (uint32_t y = 0; y < srcHeight; ++y) {
        for (uint32_t x = 0; x < srcWidth; ++x) {
            uint32_t index = y * srcWidth + x;
            uint32_t pixel = ((uint32_t *) srcPixels)[index];

            uint8_t alpha = (pixel >> 24) & 0xff;
            uint8_t b = ((pixel >> 16) & 0xff) / 4;
            uint8_t g = ((pixel >> 8) & 0xff) / 4;
            uint8_t r = (pixel & 0xff) / 4;

            uint32_t lutX = (b % 8) * 64 + r;
            uint32_t lutY = (b / 8) * 64 + g;
            uint32_t lutIndex = lutY * lutWidth + lutX;

            uint32_t lutPixel = ((uint32_t *) lutPixels)[lutIndex];
            uint8_t lutB = (lutPixel >> 16) & 0xff;
            uint8_t lutG = (lutPixel >> 8) & 0xff;
            uint8_t lutR = lutPixel & 0xff;

            auto finalR = static_cast<uint8_t>(std::round(r * 4 + intensity * (lutR - r * 4)));
            auto finalG = static_cast<uint8_t>(std::round(g * 4 + intensity * (lutG - g * 4)));
            auto finalB = static_cast<uint8_t>(std::round(b * 4 + intensity * (lutB - b * 4)));

            ((uint32_t *) outputPixels)[index] =
                    (alpha << 24) | (finalB << 16) | (finalG << 8) | finalR;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, lutBitmap);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}

bool parseCubeFile(const std::string &filename, Tensor<float, 4> &lut, int &lutSize) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        std::string line;

        int index = 0;

        while (std::getline(file, line)) {
            // Ignore comments and empty lines
            if (line.empty() || line[0] == '#' || line == "\r") {
                continue;
            }

            std::istringstream iss(line);
            std::string keyword;
            iss >> keyword;

            // Parse metadata
            if (keyword == "TITLE") {
                // Handle TITLE if necessary
            } else if (keyword == "DOMAIN_MIN") {
                // Handle DOMAIN_MIN if necessary
            } else if (keyword == "DOMAIN_MAX") {
                // Handle DOMAIN_MAX if necessary
            } else if (keyword == "LUT_3D_SIZE") {
                iss >> lutSize;
                lut = Tensor<float, 4>(lutSize, lutSize, lutSize, 3);
            } else {
                if (lutSize <= 0) {
                    return false;
                }
                // Parse LUT data
                float r, g, b;
                iss.seekg(0); // Reset stream position to the beginning
                iss >> r >> g >> b;

                // Calculate the position in the LUT
                int z = index % lutSize;
                int y = (index / lutSize) % lutSize;
                int x = index / (lutSize * lutSize);

                // Store the values in the LUT
                lut(x, y, z, 0) = r;
                lut(x, y, z, 1) = g;
                lut(x, y, z, 2) = b;
                index += 1;
            }
        }

        file.close();
        return true;
    } catch (std::exception &e) {
        return false;
    }
}


// Define the applyLUT function (reuse from the previous example)
Vector3f getColor(const Tensor<float, 4> &lut, const Vector3f &color) {
    size_t lutSize = lut.dimension(0);

    float r = color(0) * static_cast<float>((lutSize - 1));
    float g = color(1) * static_cast<float>((lutSize - 1));
    float b = color(2) * static_cast<float>((lutSize - 1));

    int r0 = static_cast<int>(r);
    int g0 = static_cast<int>(g);
    int b0 = static_cast<int>(b);

    int r1 = std::min(static_cast<int>(r0 + 1), static_cast<int>(lutSize - 1));
    int g1 = std::min(static_cast<int>(g0 + 1), static_cast<int>(lutSize - 1));
    int b1 = std::min(static_cast<int>(b0 + 1), static_cast<int>(lutSize - 1));

    float dr = r - static_cast<float>(r0);
    float dg = g - static_cast<float>(g0);
    float db = b - static_cast<float>(b0);

    Vector3f c000(lut(r0, g0, b0, 0), lut(r0, g0, b0, 1), lut(r0, g0, b0, 2));
    Vector3f c001(lut(r0, g0, b1, 0), lut(r0, g0, b1, 1), lut(r0, g0, b1, 2));
    Vector3f c010(lut(r0, g1, b0, 0), lut(r0, g1, b0, 1), lut(r0, g1, b0, 2));
    Vector3f c011(lut(r0, g1, b1, 0), lut(r0, g1, b1, 1), lut(r0, g1, b1, 2));
    Vector3f c100(lut(r1, g0, b0, 0), lut(r1, g0, b0, 1), lut(r1, g0, b0, 2));
    Vector3f c101(lut(r1, g0, b1, 0), lut(r1, g0, b1, 1), lut(r1, g0, b1, 2));
    Vector3f c110(lut(r1, g1, b0, 0), lut(r1, g1, b0, 1), lut(r1, g1, b0, 2));
    Vector3f c111(lut(r1, g1, b1, 0), lut(r1, g1, b1, 1), lut(r1, g1, b1, 2));

    Vector3f c00 = c000 * (1 - db) + c001 * db;
    Vector3f c01 = c010 * (1 - db) + c011 * db;
    Vector3f c10 = c100 * (1 - db) + c101 * db;
    Vector3f c11 = c110 * (1 - db) + c111 * db;

    Vector3f c0 = c00 * (1 - dg) + c01 * dg;
    Vector3f c1 = c10 * (1 - dg) + c11 * dg;

    Vector3f c = c0 * (1 - dr) + c1 * dr;

    return c;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_applyCubeLutImpl(
        JNIEnv *env, jobject thiz, jobject input, jstring cubeLutPath, jfloat intensity) {

    const char *lutData = env->GetStringUTFChars(cubeLutPath, nullptr);
    if (lutData == nullptr) {
        return nullptr;
    }


    int lutSize = 0;
    Tensor<float, 4> lut;

    if (!parseCubeFile((std::string(lutData)), lut, lutSize)) {
        return nullptr;
    }

    AndroidBitmapInfo srcInfo;
    void *srcPixels;
    if (AndroidBitmap_getInfo(env, input, &srcInfo) < 0) {
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, input, &srcPixels) < 0) {
        return nullptr;
    }

    uint32_t srcWidth = srcInfo.width;
    uint32_t srcHeight = srcInfo.height;

    jobject outputBitmap = createBitmap(env, srcWidth, srcHeight);
    if (outputBitmap == nullptr) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    void *outputPixels;
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    for (uint32_t y = 0; y < srcHeight; ++y) {
        for (uint32_t x = 0; x < srcWidth; ++x) {
            uint32_t index = y * srcWidth + x;
            uint32_t pixel = ((uint32_t *) srcPixels)[index];

            uint8_t alpha = (pixel >> 24) & 0xff;
            float r = ((pixel >> 16) & 0xff);
            float g = ((pixel >> 8) & 0xff);
            float b = (pixel & 0xff);

            Eigen::Vector3f prv = {r / 255.0f, g / 255.0f, b / 255.0f};
            auto color = getColor(lut, prv);

            int lutR = color.z() * 255;
            int lutG = color.y() * 255;
            int lutB = color.x() * 255;

            uint8_t finalR = static_cast<uint8_t>(std::round(
                    (r) * (1.0f - intensity) + lutR * intensity));
            uint8_t finalG = static_cast<uint8_t>(std::round(
                    (g) * (1.0f - intensity) + lutG * intensity));
            uint8_t finalB = static_cast<uint8_t>(std::round(
                    (b) * (1.0f - intensity) + lutB * intensity));

            ((uint32_t *) outputPixels)[index] =
                    (alpha << 24) | (finalR << 16) | (finalG << 8) | finalB;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}