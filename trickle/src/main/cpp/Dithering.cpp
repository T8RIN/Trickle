//
// Created by malik on 22.07.2024.
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
#include <vector>

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_ordered2By2BayerImpl(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jobject input,
                                                                           jint threshold,
                                                                           jboolean is_gray_scale) {
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

    int factor = 2;

    int matrix[2][2] = {{1, 3},
                        {4, 2}};

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
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                gray += gray * matrix[x % factor][y % factor] / 5;
                if (gray < threshold) {
                    gray = 0;
                } else {
                    gray = 255;
                }
                argb = ARGB(a, gray, gray, gray);
            } else {
                int rgb[3];
                rgb[0] = r;
                rgb[1] = g;
                rgb[2] = b;
                for (int i = 0; i <= 2; i++) {
                    int channelValue = rgb[i] + rgb[i] * matrix[x % factor][y % factor] / 5;
                    if (channelValue < threshold) {
                        rgb[i] = 0;
                    } else {
                        rgb[i] = 255;
                    }
                }
                argb = ARGB(a, rgb[0], rgb[1], rgb[2]);
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
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_ordered3By3BayerImpl(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jobject input,
                                                                           jint threshold,
                                                                           jboolean is_gray_scale) {
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

    int factor = 3;

    int matrix[3][3] = {{3, 7, 4},
                        {6, 1, 9},
                        {2, 8, 5}};

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
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                gray += gray * matrix[x % factor][y % factor] / 10;
                if (gray < threshold) {
                    gray = 0;
                } else {
                    gray = 255;
                }
                argb = ARGB(a, gray, gray, gray);
            } else {
                int rgb[3];
                rgb[0] = r;
                rgb[1] = g;
                rgb[2] = b;
                for (int i = 0; i <= 2; i++) {
                    int channelValue = rgb[i] + rgb[i] * matrix[x % factor][y % factor] / 10;
                    if (channelValue < threshold) {
                        rgb[i] = 0;
                    } else {
                        rgb[i] = 255;
                    }
                }
                argb = ARGB(a, rgb[0], rgb[1], rgb[2]);
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
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_ordered4By4BayerImpl(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jobject input,
                                                                           jint threshold,
                                                                           jboolean is_gray_scale) {
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

    int factor = 4;

    int matrix[4][4] = {{1,  9,  3,  11},
                        {13, 5,  15, 7},
                        {4,  12, 2,  10},
                        {16, 8,  14, 6}};

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
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                gray += gray * matrix[x % factor][y % factor] / 17;
                if (gray < threshold) {
                    gray = 0;
                } else {
                    gray = 255;
                }
                argb = ARGB(a, gray, gray, gray);
            } else {
                int rgb[3];
                rgb[0] = r;
                rgb[1] = g;
                rgb[2] = b;
                for (int i = 0; i <= 2; i++) {
                    int channelValue = rgb[i] + rgb[i] * matrix[x % factor][y % factor] / 17;
                    if (channelValue < threshold) {
                        rgb[i] = 0;
                    } else {
                        rgb[i] = 255;
                    }
                }
                argb = ARGB(a, rgb[0], rgb[1], rgb[2]);
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
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_ordered8By8BayerImpl(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jobject input,
                                                                           jint threshold,
                                                                           jboolean is_gray_scale) {
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

    int factor = 8;

    int matrix[8][8] = {{1,  49, 13, 61, 4,  52, 16, 64},
                        {33, 17, 45, 29, 36, 20, 48, 32},
                        {9,  57, 5,  53, 12, 60, 8,  56},
                        {41, 25, 37, 21, 44, 28, 40, 24},
                        {3,  51, 15, 63, 2,  50, 14, 62},
                        {35, 19, 47, 31, 34, 18, 46, 30},
                        {11, 59, 7,  55, 10, 58, 6,  54},
                        {43, 27, 39, 23, 42, 26, 38, 22}};

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
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                gray += gray * matrix[x % factor][y % factor] / 65;
                if (gray < threshold) {
                    gray = 0;
                } else {
                    gray = 255;
                }
                argb = ARGB(a, gray, gray, gray);
            } else {
                int rgb[3];
                rgb[0] = r;
                rgb[1] = g;
                rgb[2] = b;
                for (int i = 0; i <= 2; i++) {
                    int channelValue = rgb[i] + rgb[i] * matrix[x % factor][y % factor] / 65;
                    if (channelValue < threshold) {
                        rgb[i] = 0;
                    } else {
                        rgb[i] = 255;
                    }
                }
                argb = ARGB(a, rgb[0], rgb[1], rgb[2]);
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
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_floydSteinbergImpl(JNIEnv *env, jobject thiz,
                                                                         jobject input,
                                                                         jint threshold,
                                                                         jboolean is_gray_scale) {
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

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 1; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(resultPixels) +
                                               y * stride);

        for (uint32_t x = 1; x < width - 1; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += 7 * error / 16;
                errors[x - 1][y + 1] += 3 * error / 16;
                errors[x][y + 1] += 5 * error / 16;
                errors[x + 1][y + 1] += error / 16;
                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += 7 * redError / 16;
                redErrors[x - 1][y + 1] += 3 * redError / 16;
                redErrors[x][y + 1] += 5 * redError / 16;
                redErrors[x + 1][y + 1] += redError / 16;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += 7 * greenError / 16;
                greenErrors[x - 1][y + 1] += 3 * greenError / 16;
                greenErrors[x][y + 1] += 5 * greenError / 16;
                greenErrors[x + 1][y + 1] += greenError / 16;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += 7 * blueError / 16;
                blueErrors[x - 1][y + 1] += 3 * blueError / 16;
                blueErrors[x][y + 1] += 5 * blueError / 16;
                blueErrors[x + 1][y + 1] += blueError / 16;

                argb = ARGB(a, r, g, b);
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
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_jarvisJudiceNinkeImpl(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jobject input,
                                                                            jint threshold,
                                                                            jboolean is_gray_scale) {
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

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 2; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(resultPixels) +
                                               y * stride);

        for (uint32_t x = 2; x < width - 2; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb(a, 0, 0, 0);

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += 7 * error / 48;
                errors[x + 2][y] += 5 * error / 48;
                errors[x - 2][y + 1] += 3 * error / 48;
                errors[x - 1][y + 1] += 5 * error / 48;
                errors[x][y + 1] += 7 * error / 48;
                errors[x + 1][y + 1] += 5 * error / 48;
                errors[x + 2][y + 1] += 3 * error / 48;
                errors[x - 2][y + 2] += error / 48;
                errors[x - 1][y + 2] += 3 * error / 48;
                errors[x][y + 2] += 5 * error / 48;
                errors[x + 1][y + 2] += 3 * error / 48;
                errors[x + 2][y + 2] += error / 48;
                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += 7 * redError / 48;
                redErrors[x + 2][y] += 5 * redError / 48;
                redErrors[x - 2][y + 1] += 3 * redError / 48;
                redErrors[x - 1][y + 1] += 5 * redError / 48;
                redErrors[x][y + 1] += 7 * redError / 48;
                redErrors[x + 1][y + 1] += 5 * redError / 48;
                redErrors[x + 2][y + 1] += 3 * redError / 48;
                redErrors[x - 2][y + 2] += redError / 48;
                redErrors[x - 1][y + 2] += 3 * redError / 48;
                redErrors[x][y + 2] += 5 * redError / 48;
                redErrors[x + 1][y + 2] += 3 * redError / 48;
                redErrors[x + 2][y + 2] += redError / 48;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += 7 * greenError / 48;
                greenErrors[x + 2][y] += 5 * greenError / 48;
                greenErrors[x - 2][y + 1] += 3 * greenError / 48;
                greenErrors[x - 1][y + 1] += 5 * greenError / 48;
                greenErrors[x][y + 1] += 7 * greenError / 48;
                greenErrors[x + 1][y + 1] += 5 * greenError / 48;
                greenErrors[x + 2][y + 1] += 3 * greenError / 48;
                greenErrors[x - 2][y + 2] += greenError / 48;
                greenErrors[x - 1][y + 2] += 3 * greenError / 48;
                greenErrors[x][y + 2] += 5 * greenError / 48;
                greenErrors[x + 1][y + 2] += 3 * greenError / 48;
                greenErrors[x + 2][y + 2] += greenError / 48;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += 7 * blueError / 48;
                blueErrors[x + 2][y] += 5 * blueError / 48;
                blueErrors[x - 2][y + 1] += 3 * blueError / 48;
                blueErrors[x - 1][y + 1] += 5 * blueError / 48;
                blueErrors[x][y + 1] += 7 * blueError / 48;
                blueErrors[x + 1][y + 1] += 5 * blueError / 48;
                blueErrors[x + 2][y + 1] += 3 * blueError / 48;
                blueErrors[x - 2][y + 2] += blueError / 48;
                blueErrors[x - 1][y + 2] += 3 * blueError / 48;
                blueErrors[x][y + 2] += 5 * blueError / 48;
                blueErrors[x + 1][y + 2] += 3 * blueError / 48;
                blueErrors[x + 2][y + 2] += blueError / 48;

                argb = ARGB(a, r, g, b);
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
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_sierraImpl(JNIEnv *env, jobject thiz,
                                                                 jobject input, jint threshold,
                                                                 jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 2; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 2; x < width - 2; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += 5 * error / 32;
                errors[x + 2][y] += 3 * error / 32;
                errors[x - 2][y + 1] += 2 * error / 32;
                errors[x - 1][y + 1] += 4 * error / 32;
                errors[x][y + 1] += 5 * error / 32;
                errors[x + 1][y + 1] += 4 * error / 32;
                errors[x + 2][y + 1] += 2 * error / 32;
                errors[x - 1][y + 2] += 2 * error / 32;
                errors[x][y + 2] += 3 * error / 32;
                errors[x + 1][y + 2] += 2 * error / 32;

                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += 5 * redError / 32;
                redErrors[x + 2][y] += 3 * redError / 32;
                redErrors[x - 2][y + 1] += 2 * redError / 32;
                redErrors[x - 1][y + 1] += 4 * redError / 32;
                redErrors[x][y + 1] += 5 * redError / 32;
                redErrors[x + 1][y + 1] += 4 * redError / 32;
                redErrors[x + 2][y + 1] += 2 * redError / 32;
                redErrors[x - 1][y + 2] += 2 * redError / 32;
                redErrors[x][y + 2] += 3 * redError / 32;
                redErrors[x + 1][y + 2] += 2 * redError / 32;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += 5 * greenError / 32;
                greenErrors[x + 2][y] += 3 * greenError / 32;
                greenErrors[x - 2][y + 1] += 2 * greenError / 32;
                greenErrors[x - 1][y + 1] += 4 * greenError / 32;
                greenErrors[x][y + 1] += 5 * greenError / 32;
                greenErrors[x + 1][y + 1] += 4 * greenError / 32;
                greenErrors[x + 2][y + 1] += 2 * greenError / 32;
                greenErrors[x - 1][y + 2] += 2 * greenError / 32;
                greenErrors[x][y + 2] += 3 * greenError / 32;
                greenErrors[x + 1][y + 2] += 2 * greenError / 32;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += 5 * blueError / 32;
                blueErrors[x + 2][y] += 3 * blueError / 32;
                blueErrors[x - 2][y + 1] += 2 * blueError / 32;
                blueErrors[x - 1][y + 1] += 4 * blueError / 32;
                blueErrors[x][y + 1] += 5 * blueError / 32;
                blueErrors[x + 1][y + 1] += 4 * blueError / 32;
                blueErrors[x + 2][y + 1] += 2 * blueError / 32;
                blueErrors[x - 1][y + 2] += 2 * blueError / 32;
                blueErrors[x][y + 2] += 3 * blueError / 32;
                blueErrors[x + 1][y + 2] += 2 * blueError / 32;

                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_twoRowSierraImpl(JNIEnv *env, jobject thiz,
                                                                       jobject input,
                                                                       jint threshold,
                                                                       jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 1; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 2; x < width - 2; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += 4 * error / 16;
                errors[x + 2][y] += 3 * error / 16;
                errors[x - 2][y + 1] += error / 16;
                errors[x - 1][y + 1] += 2 * error / 16;
                errors[x][y + 1] += 3 * error / 16;
                errors[x + 1][y + 1] += 2 * error / 16;
                errors[x + 2][y + 1] += error / 16;

                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += 4 * redError / 16;
                redErrors[x + 2][y] += 3 * redError / 16;
                redErrors[x - 2][y + 1] += redError / 16;
                redErrors[x - 1][y + 1] += 2 * redError / 16;
                redErrors[x][y + 1] += 3 * redError / 16;
                redErrors[x + 1][y + 1] += 2 * redError / 16;
                redErrors[x + 2][y + 1] += redError / 16;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += 4 * greenError / 16;
                greenErrors[x + 2][y] += 3 * greenError / 16;
                greenErrors[x - 2][y + 1] += greenError / 16;
                greenErrors[x - 1][y + 1] += 2 * greenError / 16;
                greenErrors[x][y + 1] += 3 * greenError / 16;
                greenErrors[x + 1][y + 1] += 2 * greenError / 16;
                greenErrors[x + 2][y + 1] += greenError / 16;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += 4 * blueError / 16;
                blueErrors[x + 2][y] += 3 * blueError / 16;
                blueErrors[x - 2][y + 1] += blueError / 16;
                blueErrors[x - 1][y + 1] += 2 * blueError / 16;
                blueErrors[x][y + 1] += 3 * blueError / 16;
                blueErrors[x + 1][y + 1] += 2 * blueError / 16;
                blueErrors[x + 2][y + 1] += blueError / 16;

                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_sierraLiteImpl(JNIEnv *env, jobject thiz,
                                                                     jobject input, jint threshold,
                                                                     jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 1; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 2; x < width - 2; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += 4 * error / 16;
                errors[x + 2][y] += 3 * error / 16;
                errors[x - 2][y + 1] += error / 16;
                errors[x - 1][y + 1] += 2 * error / 16;
                errors[x][y + 1] += 3 * error / 16;
                errors[x + 1][y + 1] += 2 * error / 16;
                errors[x + 2][y + 1] += error / 16;

                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += 4 * redError / 16;
                redErrors[x + 2][y] += 3 * redError / 16;
                redErrors[x - 2][y + 1] += redError / 16;
                redErrors[x - 1][y + 1] += 2 * redError / 16;
                redErrors[x][y + 1] += 3 * redError / 16;
                redErrors[x + 1][y + 1] += 2 * redError / 16;
                redErrors[x + 2][y + 1] += redError / 16;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += 4 * greenError / 16;
                greenErrors[x + 2][y] += 3 * greenError / 16;
                greenErrors[x - 2][y + 1] += greenError / 16;
                greenErrors[x - 1][y + 1] += 2 * greenError / 16;
                greenErrors[x][y + 1] += 3 * greenError / 16;
                greenErrors[x + 1][y + 1] += 2 * greenError / 16;
                greenErrors[x + 2][y + 1] += greenError / 16;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += 4 * blueError / 16;
                blueErrors[x + 2][y] += 3 * blueError / 16;
                blueErrors[x - 2][y + 1] += blueError / 16;
                blueErrors[x - 1][y + 1] += 2 * blueError / 16;
                blueErrors[x][y + 1] += 3 * blueError / 16;
                blueErrors[x + 1][y + 1] += 2 * blueError / 16;
                blueErrors[x + 2][y + 1] += blueError / 16;

                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_atkinsonImpl(JNIEnv *env, jobject thiz,
                                                                   jobject input, jint threshold,
                                                                   jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 2; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 1; x < width - 2; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += error / 8;
                errors[x + 2][y] += error / 8;
                errors[x - 1][y + 1] += error / 8;
                errors[x][y + 1] += error / 8;
                errors[x + 1][y + 1] += error / 8;
                errors[x][y + 2] += error / 8;

                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += redError / 8;
                redErrors[x + 2][y] += redError / 8;
                redErrors[x - 1][y + 1] += redError / 8;
                redErrors[x][y + 1] += redError / 8;
                redErrors[x + 1][y + 1] += redError / 8;
                redErrors[x][y + 2] += redError / 8;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += greenError / 8;
                greenErrors[x + 2][y] += greenError / 8;
                greenErrors[x - 1][y + 1] += greenError / 8;
                greenErrors[x][y + 1] += greenError / 8;
                greenErrors[x + 1][y + 1] += greenError / 8;
                greenErrors[x][y + 2] += greenError / 8;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += blueError / 8;
                blueErrors[x + 2][y] += blueError / 8;
                blueErrors[x - 1][y + 1] += blueError / 8;
                blueErrors[x][y + 1] += blueError / 8;
                blueErrors[x + 1][y + 1] += blueError / 8;
                blueErrors[x][y + 2] += blueError / 8;

                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_stuckiImpl(JNIEnv *env, jobject thiz,
                                                                 jobject input, jint threshold,
                                                                 jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 2; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 2; x < width - 2; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += 8 * error / 42;
                errors[x + 2][y] += 4 * error / 42;
                errors[x - 2][y + 1] += 2 * error / 42;
                errors[x - 1][y + 1] += 4 * error / 42;
                errors[x][y + 1] += 8 * error / 42;
                errors[x + 1][y + 1] += 4 * error / 42;
                errors[x + 2][y + 1] += 2 * error / 42;
                errors[x - 2][y + 2] += error / 42;
                errors[x - 1][y + 2] += 2 * error / 42;
                errors[x][y + 2] += 4 * error / 42;
                errors[x + 1][y + 2] += 2 * error / 42;
                errors[x + 2][y + 2] += error / 42;

                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += 8 * redError / 42;
                redErrors[x + 2][y] += 4 * redError / 42;
                redErrors[x - 2][y + 1] += 2 * redError / 42;
                redErrors[x - 1][y + 1] += 4 * redError / 42;
                redErrors[x][y + 1] += 8 * redError / 42;
                redErrors[x + 1][y + 1] += 4 * redError / 42;
                redErrors[x + 2][y + 1] += 2 * redError / 42;
                redErrors[x - 2][y + 2] += redError / 42;
                redErrors[x - 1][y + 2] += 2 * redError / 42;
                redErrors[x][y + 2] += 4 * redError / 42;
                redErrors[x + 1][y + 2] += 2 * redError / 42;
                redErrors[x + 2][y + 2] += redError / 42;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += 8 * greenError / 42;
                greenErrors[x + 2][y] += 4 * greenError / 42;
                greenErrors[x - 2][y + 1] += 2 * greenError / 42;
                greenErrors[x - 1][y + 1] += 4 * greenError / 42;
                greenErrors[x][y + 1] += 8 * greenError / 42;
                greenErrors[x + 1][y + 1] += 4 * greenError / 42;
                greenErrors[x + 2][y + 1] += 2 * greenError / 42;
                greenErrors[x - 2][y + 2] += greenError / 42;
                greenErrors[x - 1][y + 2] += 2 * greenError / 42;
                greenErrors[x][y + 2] += 4 * greenError / 42;
                greenErrors[x + 1][y + 2] += 2 * greenError / 42;
                greenErrors[x + 2][y + 2] += greenError / 42;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += 8 * blueError / 42;
                blueErrors[x + 2][y] += 4 * blueError / 42;
                blueErrors[x - 2][y + 1] += 2 * blueError / 42;
                blueErrors[x - 1][y + 1] += 4 * blueError / 42;
                blueErrors[x][y + 1] += 8 * blueError / 42;
                blueErrors[x + 1][y + 1] += 4 * blueError / 42;
                blueErrors[x + 2][y + 1] += 2 * blueError / 42;
                blueErrors[x - 2][y + 2] += blueError / 42;
                blueErrors[x - 1][y + 2] += 2 * blueError / 42;
                blueErrors[x][y + 2] += 4 * blueError / 42;
                blueErrors[x + 1][y + 2] += 2 * blueError / 42;
                blueErrors[x + 2][y + 2] += blueError / 42;

                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_burkesImpl(JNIEnv *env, jobject thiz,
                                                                 jobject input, jint threshold,
                                                                 jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 1; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 2; x < width - 2; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += 8 * error / 32;
                errors[x + 2][y] += 4 * error / 32;
                errors[x - 2][y + 1] += 2 * error / 32;
                errors[x - 1][y + 1] += 4 * error / 32;
                errors[x][y + 1] += 8 * error / 32;
                errors[x + 1][y + 1] += 4 * error / 32;
                errors[x + 2][y + 1] += 2 * error / 32;

                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += 8 * redError / 32;
                redErrors[x + 2][y] += 4 * redError / 32;
                redErrors[x - 2][y + 1] += 2 * redError / 32;
                redErrors[x - 1][y + 1] += 4 * redError / 32;
                redErrors[x][y + 1] += 8 * redError / 32;
                redErrors[x + 1][y + 1] += 4 * redError / 32;
                redErrors[x + 2][y + 1] += 2 * redError / 32;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += 8 * greenError / 32;
                greenErrors[x + 2][y] += 4 * greenError / 32;
                greenErrors[x - 2][y + 1] += 2 * greenError / 32;
                greenErrors[x - 1][y + 1] += 4 * greenError / 32;
                greenErrors[x][y + 1] += 8 * greenError / 32;
                greenErrors[x + 1][y + 1] += 4 * greenError / 32;
                greenErrors[x + 2][y + 1] += 2 * greenError / 32;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += 8 * blueError / 32;
                blueErrors[x + 2][y] += 4 * blueError / 32;
                blueErrors[x - 2][y + 1] += 2 * blueError / 32;
                blueErrors[x - 1][y + 1] += 4 * blueError / 32;
                blueErrors[x][y + 1] += 8 * blueError / 32;
                blueErrors[x + 1][y + 1] += 4 * blueError / 32;
                blueErrors[x + 2][y + 1] += 2 * blueError / 32;

                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_falseFloydSteinbergImpl(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jobject input,
                                                                              jint threshold,
                                                                              jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }

    std::vector<std::vector<int>> errors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> redErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> greenErrors(width, std::vector<int>(height, 0));
    std::vector<std::vector<int>> blueErrors(width, std::vector<int>(height, 0));

    for (uint32_t y = 0; y < height - 1; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 1; x < width - 1; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int error;
                if (gray + errors[x][y] < threshold) {
                    error = gray + errors[x][y];
                    gray = 0;
                } else {
                    error = gray + errors[x][y] - 255;
                    gray = 255;
                }
                errors[x + 1][y] += 3 * error / 8;
                errors[x][y + 1] += 3 * error / 8;
                errors[x + 1][y + 1] += 2 * error / 8;

                argb = ARGB(a, gray, gray, gray);
            } else {
                int redError, greenError, blueError;

                if (r + redErrors[x][y] < threshold) {
                    redError = r + redErrors[x][y];
                    r = 0;
                } else {
                    redError = r + redErrors[x][y] - 255;
                    r = 255;
                }
                redErrors[x + 1][y] += 3 * redError / 8;
                redErrors[x][y + 1] += 3 * redError / 8;
                redErrors[x + 1][y + 1] += 2 * redError / 8;

                if (g + greenErrors[x][y] < threshold) {
                    greenError = g + greenErrors[x][y];
                    g = 0;
                } else {
                    greenError = g + greenErrors[x][y] - 255;
                    g = 255;
                }
                greenErrors[x + 1][y] += 3 * greenError / 8;
                greenErrors[x][y + 1] += 3 * greenError / 8;
                greenErrors[x + 1][y + 1] += 2 * greenError / 8;

                if (b + blueErrors[x][y] < threshold) {
                    blueError = b + blueErrors[x][y];
                    b = 0;
                } else {
                    blueError = b + blueErrors[x][y] - 255;
                    b = 255;
                }
                blueErrors[x + 1][y] += 3 * blueError / 8;
                blueErrors[x][y + 1] += 3 * blueError / 8;
                blueErrors[x + 1][y + 1] += 2 * blueError / 8;

                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_simpleLeftToRightErrorDiffusionImpl(
        JNIEnv *env, jobject thiz, jobject input, jint threshold, jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, outputBitmap);
        return nullptr;
    }

    for (uint32_t y = 0; y < height; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        int error = 0;
        int redError = 0;
        int greenError = 0;
        int blueError = 0;

        for (uint32_t x = 0; x < width; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int delta;
                if (gray + error < threshold) {
                    delta = gray;
                    gray = 0;
                } else {
                    delta = gray - 255;
                    gray = 255;
                }
                if (abs(delta) < 10) delta = 0;
                error += delta;

                argb = ARGB(a, gray, gray, gray);
            } else {
                int redDelta, greenDelta, blueDelta;

                if (r + redError < threshold) {
                    redDelta = r;
                    r = 0;
                } else {
                    redDelta = r - 255;
                    r = 255;
                }
                if (abs(redDelta) < 10) redDelta = 0;
                redError += redDelta;

                if (g + greenError < threshold) {
                    greenDelta = g;
                    g = 0;
                } else {
                    greenDelta = g - 255;
                    g = 255;
                }
                if (abs(greenDelta) < 10) greenDelta = 0;
                greenError += greenDelta;

                if (b + blueError < threshold) {
                    blueDelta = b;
                    b = 0;
                } else {
                    blueDelta = b - 255;
                    b = 255;
                }
                if (abs(blueDelta) < 10) blueDelta = 0;
                blueError += blueDelta;

                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_randomImpl(JNIEnv *env, jobject thiz,
                                                                 jobject input,
                                                                 jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (uint32_t y = 0; y < height; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 0; x < width; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                int threshold = std::rand() % 256;
                gray = gray < threshold ? 0 : 255;
                argb = ARGB(a, gray, gray, gray);
            } else {
                int threshold = std::rand() % 256;
                r = r < threshold ? 0 : 255;
                g = g < threshold ? 0 : 255;
                b = b < threshold ? 0 : 255;
                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_simpleThresholdImpl(JNIEnv *env, jobject thiz,
                                                                          jobject input,
                                                                          jint threshold,
                                                                          jboolean is_gray_scale) {
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

    jobject outputBitmap = createBitmap(env, width, height);

    AndroidBitmapInfo outputInfo;
    void *outputPixels;
    if (AndroidBitmap_getInfo(env, outputBitmap, &outputInfo) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    for (uint32_t y = 0; y < height; ++y) {
        auto src = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(sourcePixels) +
                                               y * stride);
        auto dst = reinterpret_cast<uint8_t *>(reinterpret_cast<uint8_t *>(outputPixels) +
                                               y * stride);

        for (uint32_t x = 0; x < width; ++x) {
            int r = src[0];
            int g = src[1];
            int b = src[2];
            int a = src[3];

            ARGB argb;

            if (is_gray_scale) {
                int gray = r;
                gray = gray < threshold ? 0 : 255;
                argb = ARGB(a, gray, gray, gray);
            } else {
                r = r < threshold ? 0 : 255;
                g = g < threshold ? 0 : 255;
                b = b < threshold ? 0 : 255;
                argb = ARGB(a, r, g, b);
            }

            dst[0] = argb.r;
            dst[1] = argb.g;
            dst[2] = argb.b;
            dst[3] = argb.a;

            dst += 4;
            src += 4;
        }
    }

    AndroidBitmap_unlockPixels(env, input);
    AndroidBitmap_unlockPixels(env, outputBitmap);

    return outputBitmap;
}