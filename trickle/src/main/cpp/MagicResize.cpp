#include <android/bitmap.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <new>
#include <thread>
#include <vector>

#include "BitmapUtils.h"

namespace {

// Kernel definitions adapted from A.R. Viddeleer's MIT-0 implementations:
// https://github.com/Viddeleer/Image-Processing
enum class MagicResizeType {
    Classic = 0,
    Sharp2013 = 1,
    Sharp2021 = 2
};

struct Pixel {
    float red;
    float green;
    float blue;
    float alpha;
};

struct Contribution {
    int index;
    double weight;
};

using Contributions = std::vector<std::vector<Contribution>>;

double kernelRadius(MagicResizeType type) {
    switch (type) {
        case MagicResizeType::Classic:
            return 1.5;
        case MagicResizeType::Sharp2013:
            return 2.5;
        case MagicResizeType::Sharp2021:
            return 4.5;
    }
    return 0.0;
}

double kernel(double x, MagicResizeType type) {
    x = std::abs(x);

    switch (type) {
        case MagicResizeType::Classic:
            if (x <= 0.5) return 0.75 - x * x;
            if (x < 1.5) return 0.5 * (x - 1.5) * (x - 1.5);
            return 0.0;
        case MagicResizeType::Sharp2013:
            if (x <= 0.5) return 17.0 / 16.0 - 7.0 / 4.0 * x * x;
            if (x <= 1.5) return 0.25 * (4.0 * x * x - 11.0 * x + 7.0);
            if (x <= 2.5) return -0.125 * (x - 2.5) * (x - 2.5);
            return 0.0;
        case MagicResizeType::Sharp2021:
            if (x <= 0.5) return 577.0 / 576.0 - 239.0 / 144.0 * x * x;
            if (x <= 1.5) return (140.0 * x * x - 379.0 * x + 239.0) / 144.0;
            if (x <= 2.5) return -(24.0 * x * x - 113.0 * x + 130.0) / 144.0;
            if (x <= 3.5) return (4.0 * x * x - 27.0 * x + 45.0) / 144.0;
            if (x <= 4.5) return -(2.0 * x - 9.0) * (2.0 * x - 9.0) / 1152.0;
            return 0.0;
    }
    return 0.0;
}

float srgbToLinear(float value) {
    if (value <= 0.04045f) return value / 12.92f;
    return std::pow((value + 0.055f) / 1.055f, 2.4f);
}

float linearToSrgb(float value) {
    if (value <= 0.0031308f) return value * 12.92f;
    return 1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f;
}

Contributions buildContributions(int sourceSize, int targetSize, MagicResizeType type) {
    Contributions result(static_cast<size_t>(targetSize));
    const double ratio = static_cast<double>(sourceSize) / targetSize;
    const double scale = std::max(1.0, ratio);
    const double radius = kernelRadius(type);
    const int footprint = static_cast<int>(std::ceil(radius * scale));

    for (int target = 0; target < targetSize; ++target) {
        const double sourcePosition = (target + 0.5) * ratio - 0.5;
        const int sourceCenter = static_cast<int>(std::floor(sourcePosition));
        const double fraction = sourcePosition - sourceCenter;
        double weightSum = 0.0;

        auto &items = result[static_cast<size_t>(target)];
        items.reserve(static_cast<size_t>(footprint * 2 + 1));
        for (int offset = -footprint; offset <= footprint; ++offset) {
            const int source = sourceCenter + offset;
            if (source < 0 || source >= sourceSize) continue;

            const double distance = std::abs(fraction - offset) / scale;
            if (distance >= radius) continue;

            const double weight = kernel(distance, type);
            items.push_back({source, weight});
            weightSum += weight;
        }

        if (std::abs(weightSum) > std::numeric_limits<double>::epsilon()) {
            for (auto &item : items) item.weight /= weightSum;
        }
    }

    return result;
}

template<typename Work>
void parallelFor(int count, Work work) {
    const unsigned int availableThreads = std::thread::hardware_concurrency();
    const int threadCount = std::min(
        count,
        static_cast<int>(std::min(availableThreads == 0 ? 1U : availableThreads, 8U))
    );
    if (threadCount <= 1 || count < 16) {
        work(0, count);
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(threadCount));
    int createdThreads = 0;
    try {
        for (; createdThreads < threadCount; ++createdThreads) {
            const int from = createdThreads * count / threadCount;
            const int to = (createdThreads + 1) * count / threadCount;
            threads.emplace_back([=, &work]() { work(from, to); });
        }
    } catch (...) {
        work(createdThreads * count / threadCount, count);
    }
    for (auto &thread : threads) thread.join();
}

Pixel filterPixels(const Pixel *pixels, const std::vector<Contribution> &contributions) {
    Pixel result = {0.0f, 0.0f, 0.0f, 0.0f};
    for (const auto &item : contributions) {
        const Pixel &pixel = pixels[item.index];
        result.red += static_cast<float>(pixel.red * item.weight);
        result.green += static_cast<float>(pixel.green * item.weight);
        result.blue += static_cast<float>(pixel.blue * item.weight);
        result.alpha += static_cast<float>(pixel.alpha * item.weight);
    }
    return result;
}

uint8_t toByte(float value) {
    return static_cast<uint8_t>(std::round(std::clamp(value, 0.0f, 1.0f) * 255.0f));
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_ResizePipelineImpl_magicResizeImpl(
    JNIEnv *env,
    jobject,
    jobject input,
    jint targetWidth,
    jint targetHeight,
    jint typeValue
) {
    if (targetWidth <= 0 || targetHeight <= 0 || typeValue < 0 || typeValue > 2) return nullptr;

    AndroidBitmapInfo sourceInfo;
    if (AndroidBitmap_getInfo(env, input, &sourceInfo) != ANDROID_BITMAP_RESULT_SUCCESS ||
        sourceInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888 ||
        sourceInfo.width > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
        sourceInfo.height > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
        return nullptr;
    }

    void *sourcePixelsRaw = nullptr;
    if (AndroidBitmap_lockPixels(env, input, &sourcePixelsRaw) != ANDROID_BITMAP_RESULT_SUCCESS) {
        return nullptr;
    }

    jobject output = nullptr;
    try {
        const int sourceWidth = static_cast<int>(sourceInfo.width);
        const int sourceHeight = static_cast<int>(sourceInfo.height);
        const auto type = static_cast<MagicResizeType>(typeValue);
        std::vector<Pixel> source(static_cast<size_t>(sourceWidth) * sourceHeight);

        for (int y = 0; y < sourceHeight; ++y) {
            const auto *row = static_cast<const uint8_t *>(sourcePixelsRaw) +
                static_cast<size_t>(y) * sourceInfo.stride;
            for (int x = 0; x < sourceWidth; ++x) {
                const uint8_t *rgba = row + x * 4;
                const float alpha = rgba[3] / 255.0f;
                const float alphaScale = alpha > 0.0f ? 1.0f / (255.0f * alpha) : 0.0f;
                Pixel &pixel = source[static_cast<size_t>(y) * sourceWidth + x];
                pixel.red = srgbToLinear(rgba[0] * alphaScale) * alpha;
                pixel.green = srgbToLinear(rgba[1] * alphaScale) * alpha;
                pixel.blue = srgbToLinear(rgba[2] * alphaScale) * alpha;
                pixel.alpha = alpha;
            }
        }

        const Contributions horizontalWeights = buildContributions(sourceWidth, targetWidth, type);
        const Contributions verticalWeights = buildContributions(sourceHeight, targetHeight, type);
        std::vector<Pixel> horizontal(static_cast<size_t>(targetWidth) * sourceHeight);

        parallelFor(sourceHeight, [&](int from, int to) {
            for (int y = from; y < to; ++y) {
                const Pixel *sourceRow = source.data() + static_cast<size_t>(y) * sourceWidth;
                Pixel *targetRow = horizontal.data() + static_cast<size_t>(y) * targetWidth;
                for (int x = 0; x < targetWidth; ++x) {
                    targetRow[x] = filterPixels(sourceRow, horizontalWeights[static_cast<size_t>(x)]);
                }
            }
        });

        output = createBitmap(env, static_cast<uint32_t>(targetWidth), static_cast<uint32_t>(targetHeight));
        if (output != nullptr && !env->ExceptionCheck()) {
            AndroidBitmapInfo outputInfo;
            void *outputPixelsRaw = nullptr;
            if (AndroidBitmap_getInfo(env, output, &outputInfo) == ANDROID_BITMAP_RESULT_SUCCESS &&
                AndroidBitmap_lockPixels(env, output, &outputPixelsRaw) == ANDROID_BITMAP_RESULT_SUCCESS) {
                parallelFor(targetHeight, [&](int from, int to) {
                    for (int y = from; y < to; ++y) {
                        auto *row = static_cast<uint8_t *>(outputPixelsRaw) +
                            static_cast<size_t>(y) * outputInfo.stride;
                        const auto &weights = verticalWeights[static_cast<size_t>(y)];
                        for (int x = 0; x < targetWidth; ++x) {
                            Pixel pixel = {0.0f, 0.0f, 0.0f, 0.0f};
                            for (const auto &item : weights) {
                                const Pixel &sample = horizontal[
                                    static_cast<size_t>(item.index) * targetWidth + x
                                ];
                                pixel.red += static_cast<float>(sample.red * item.weight);
                                pixel.green += static_cast<float>(sample.green * item.weight);
                                pixel.blue += static_cast<float>(sample.blue * item.weight);
                                pixel.alpha += static_cast<float>(sample.alpha * item.weight);
                            }

                            const float alpha = std::clamp(pixel.alpha, 0.0f, 1.0f);
                            const float inverseAlpha = alpha > 0.0f ? 1.0f / alpha : 0.0f;
                            uint8_t *rgba = row + x * 4;
                            rgba[0] = toByte(linearToSrgb(pixel.red * inverseAlpha) * alpha);
                            rgba[1] = toByte(linearToSrgb(pixel.green * inverseAlpha) * alpha);
                            rgba[2] = toByte(linearToSrgb(pixel.blue * inverseAlpha) * alpha);
                            rgba[3] = toByte(alpha);
                        }
                    }
                });
                AndroidBitmap_unlockPixels(env, output);
            } else {
                output = nullptr;
            }
        }
    } catch (const std::bad_alloc &) {
        output = nullptr;
    }

    AndroidBitmap_unlockPixels(env, input);
    return output;
}
