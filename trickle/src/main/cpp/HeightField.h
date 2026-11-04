#pragma once

#include <vector>
#include "PhotoEffects.h"

namespace photo {

class HeightField {
public:
    int width;
    int height;
    std::vector<float> values;

    HeightField(int width, int height) : width(width), height(height), values(size_t(width) * height) {}

    explicit HeightField(const Image &image, int limit = 768)
            : HeightField(std::max(1, int(image.width * std::min(1.f, float(limit) / std::max(image.width, image.height)))),
                          std::max(1, int(image.height * std::min(1.f, float(limit) / std::max(image.width, image.height))))) {
        const float scaleX = float(image.width) / width;
        const float scaleY = float(image.height) / height;
        for (int y = 0; y < height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < width; ++x) {
                const float px = (x + .5f) * scaleX - .5f;
                const float py = (y + .5f) * scaleY - .5f;
                const float dx = std::max(0.f, scaleX - 1.f) * .25f;
                const float dy = std::max(0.f, scaleY - 1.f) * .25f;
                at(x, y) = .25f * (luma(image.sample(px - dx, py - dy)) +
                                  luma(image.sample(px + dx, py - dy)) +
                                  luma(image.sample(px - dx, py + dy)) +
                                  luma(image.sample(px + dx, py + dy)));
            }
        }
    }

    float &at(int x, int y) {
        return values[size_t(std::clamp(y, 0, height - 1)) * width + std::clamp(x, 0, width - 1)];
    }

    float at(int x, int y) const {
        return values[size_t(std::clamp(y, 0, height - 1)) * width + std::clamp(x, 0, width - 1)];
    }

    float sample(float x, float y) const {
        x = std::clamp(x, 0.f, float(width - 1));
        y = std::clamp(y, 0.f, float(height - 1));
        const int ix = int(x), iy = int(y);
        return mix(mix(at(ix, iy), at(ix + 1, iy), x - ix),
                   mix(at(ix, iy + 1), at(ix + 1, iy + 1), x - ix), y - iy);
    }

    float fromImage(int x, int y, const Image &image) const {
        return sample((x + .5f) * width / image.width - .5f,
                      (y + .5f) * height / image.height - .5f);
    }

    float dx(int x, int y) const { return .5f * (at(x + 1, y) - at(x - 1, y)); }
    float dy(int x, int y) const { return .5f * (at(x, y + 1) - at(x, y - 1)); }

    void blur(int radius) {
        radius = std::clamp(radius, 0, std::max(width, height));
        if (!radius) return;
        std::vector<float> temporary(values.size());
        const float divisor = 1.f / (2 * radius + 1);
        for (int y = 0; y < height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            float sum = 0.f;
            for (int k = -radius; k <= radius; ++k) sum += at(k, y);
            for (int x = 0; x < width; ++x) {
                temporary[size_t(y) * width + x] = sum * divisor;
                sum += at(x + radius + 1, y) - at(x - radius, y);
            }
        }
        for (int x = 0; x < width; ++x) {
            if (EncodingInterruption::currentRequested()) return;
            const auto read = [&](int y) {
                return temporary[size_t(std::clamp(y, 0, height - 1)) * width + x];
            };
            float sum = 0.f;
            for (int k = -radius; k <= radius; ++k) sum += read(k);
            for (int y = 0; y < height; ++y) {
                at(x, y) = sum * divisor;
                sum += read(y + radius + 1) - read(y - radius);
            }
        }
    }
};

}
