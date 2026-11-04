#pragma once

#include <vector>
#include "PhotoEffects.h"

namespace photo {

// Bounded scalar workspace for image-analysis transforms.
struct AnalysisImage {
    int width;
    int height;
    std::vector<float> pixels;

    AnalysisImage(const Image &image, int maxSide) {
        const float scale = std::min(1.f, float(maxSide) / std::max(image.width, image.height));
        width = std::max(1, int(std::round(image.width * scale)));
        height = std::max(1, int(std::round(image.height * scale)));
        pixels.resize(size_t(width) * height);
        const double stepX = double(image.width) / width;
        const double stepY = double(image.height) / height;
        size_t samples = 0;
        for (int y = 0; y < height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            const double top = y * stepY, bottom = (y + 1) * stepY;
            const int firstRow = int(top);
            const int lastRow = std::min(image.height - 1, int(std::ceil(bottom)) - 1);
            for (int x = 0; x < width; ++x) {
                const double left = x * stepX, right = (x + 1) * stepX;
                const int firstColumn = int(left);
                const int lastColumn = std::min(image.width - 1, int(std::ceil(right)) - 1);
                double sum = 0;
                for (int sy = firstRow; sy <= lastRow; ++sy) {
                    const double weightY = std::min(bottom, double(sy + 1)) - std::max(top, double(sy));
                    for (int sx = firstColumn; sx <= lastColumn; ++sx) {
                        if ((samples++ & 8191) == 0 && EncodingInterruption::currentRequested()) return;
                        const double weightX = std::min(right, double(sx + 1)) - std::max(left, double(sx));
                        sum += luma(image.read(sx, sy)) * weightX * weightY;
                    }
                }
                pixels[size_t(y) * width + x] = float(sum / (stepX * stepY));
            }
        }
    }

    float sample(float x, float y) const {
        x = std::clamp(x, 0.f, float(width - 1));
        y = std::clamp(y, 0.f, float(height - 1));
        const int x0 = int(x), y0 = int(y);
        const int x1 = std::min(x0 + 1, width - 1), y1 = std::min(y0 + 1, height - 1);
        return mix(mix(pixels[size_t(y0) * width + x0], pixels[size_t(y0) * width + x1], x - x0),
                   mix(pixels[size_t(y1) * width + x0], pixels[size_t(y1) * width + x1], x - x0), y - y0);
    }
};

}
