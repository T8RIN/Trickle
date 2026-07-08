/*
 * AutoWhiteBalanceFilter.cpp
 */

#include "AutoWhiteBalanceFilter.h"
#include <algorithm>
#include <vector>

AutoWhiteBalanceFilter::AutoWhiteBalanceFilter(int *pixels, int width, int height,
        AutoWhiteBalanceOptions options) :
        ImageFilter(pixels, width, height), options(options) {
}

int *AutoWhiteBalanceFilter::procImage() {
    if (pixels == nullptr || width <= 0 || height <= 0) {
        return pixels;
    }

    int histR[256] = {0};
    int histG[256] = {0};
    int histB[256] = {0};
    long long validPixelCount = 0;

    uint8_t *bytePixels = reinterpret_cast<uint8_t *>(pixels);

    // 1. Build histograms, ignoring alpha == 0
    for (int i = 0; i < width * height; ++i) {
        uint8_t r = bytePixels[i * 4 + 0];
        uint8_t g = bytePixels[i * 4 + 1];
        uint8_t b = bytePixels[i * 4 + 2];
        uint8_t a = bytePixels[i * 4 + 3];

        if (a > 0) {
            histR[r]++;
            histG[g]++;
            histB[b]++;
            validPixelCount++;
        }
    }

    if (validPixelCount == 0) {
        return pixels;
    }

    float clip = std::max(0.0f, std::min(10.0f, options.clipPercent));
    float strength = std::max(0.0f, std::min(1.0f, options.strength));
    long long cut = static_cast<long long>(validPixelCount * clip / 100.0f);

    uint8_t lutR[256], lutG[256], lutB[256];

    auto calculateLUT = [&](int *hist, uint8_t *lut) {
        int low = 0;
        long long count = 0;
        for (; low < 256; ++low) {
            count += hist[low];
            if (count > cut) break;
        }

        int high = 255;
        count = 0;
        for (; high >= 0; --high) {
            count += hist[high];
            if (count > cut) break;
        }

        if (high <= low) {
            for (int i = 0; i < 256; ++i) lut[i] = i;
        } else {
            float range = static_cast<float>(high - low);
            for (int i = 0; i < 256; ++i) {
                if (i <= low) lut[i] = 0;
                else if (i >= high) lut[i] = 255;
                else {
                    lut[i] = static_cast<uint8_t>((i - low) * 255.0f / range + 0.5f);
                }
            }
        }
    };

    calculateLUT(histR, lutR);
    calculateLUT(histG, lutG);
    calculateLUT(histB, lutB);

    // 2. Apply LUT and blend with strength
    for (int i = 0; i < width * height; ++i) {
        uint8_t a = bytePixels[i * 4 + 3];
        if (a == 0) continue;

        uint8_t r = bytePixels[i * 4 + 0];
        uint8_t g = bytePixels[i * 4 + 1];
        uint8_t b = bytePixels[i * 4 + 2];

        uint8_t balancedR = lutR[r];
        uint8_t balancedG = lutG[g];
        uint8_t balancedB = lutB[b];

        bytePixels[i * 4 + 0] = static_cast<uint8_t>(r + (balancedR - r) * strength + 0.5f);
        bytePixels[i * 4 + 1] = static_cast<uint8_t>(g + (balancedG - g) * strength + 0.5f);
        bytePixels[i * 4 + 2] = static_cast<uint8_t>(b + (balancedB - b) * strength + 0.5f);
        // Alpha is preserved by not changing bytePixels[i * 4 + 3]
    }

    return pixels;
}
