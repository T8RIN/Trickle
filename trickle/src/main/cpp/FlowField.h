#pragma once

#include "HeightField.h"

namespace photo {

struct FlowDirection {
    float x;
    float y;

    float dot(FlowDirection other) const { return x * other.x + y * other.y; }
    FlowDirection operator*(float scale) const { return {x * scale, y * scale}; }
    FlowDirection operator+(FlowDirection other) const { return {x + other.x, y + other.y}; }

    FlowDirection normalized() const {
        const float length = std::sqrt(x * x + y * y);
        return length > 1e-6f ? *this * (1.f / length) : FlowDirection{1.f, 0.f};
    }
};

class FlowField {
public:
    HeightField luminance;

    explicit FlowField(const Image &image) : luminance(image),
            horizontal(luminance.width, luminance.height), vertical(luminance.width, luminance.height) {
        HeightField smoothed = luminance;
        smoothed.blur(1);
        HeightField xx(luminance.width, luminance.height);
        HeightField xy(luminance.width, luminance.height);
        HeightField yy(luminance.width, luminance.height);
        HeightField magnitude(luminance.width, luminance.height);
        for (int y = 0; y < luminance.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < luminance.width; ++x) {
                const float gx = smoothed.dx(x, y), gy = smoothed.dy(x, y);
                xx.at(x, y) = gx * gx;
                xy.at(x, y) = gx * gy;
                yy.at(x, y) = gy * gy;
                magnitude.at(x, y) = std::sqrt(gx * gx + gy * gy);
            }
        }
        const int radius = std::max(1, std::max(luminance.width, luminance.height) / 256);
        xx.blur(radius);
        xy.blur(radius);
        yy.blur(radius);
        for (int y = 0; y < luminance.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < luminance.width; ++x) {
                const float angle = .5f * std::atan2(2.f * xy.at(x, y), xx.at(x, y) - yy.at(x, y)) + .5f * pi;
                horizontal.at(x, y) = std::cos(angle);
                vertical.at(x, y) = std::sin(angle);
            }
        }
        HeightField nextX(luminance.width, luminance.height);
        HeightField nextY(luminance.width, luminance.height);
        for (int iteration = 0; iteration < 2; ++iteration) {
            for (int y = 0; y < luminance.height; ++y) {
                if (EncodingInterruption::currentRequested()) return;
                for (int x = 0; x < luminance.width; ++x) {
                    if ((x & 63) == 0 && EncodingInterruption::currentRequested()) return;
                    const FlowDirection center{horizontal.at(x, y), vertical.at(x, y)};
                    FlowDirection sum{};
                    for (int dy = -2; dy <= 2; ++dy) {
                        for (int dx = -2; dx <= 2; ++dx) {
                            const FlowDirection neighbor{horizontal.at(x + dx, y + dy), vertical.at(x + dx, y + dy)};
                            const float alignment = center.dot(neighbor);
                            const float weight = std::abs(alignment) / (1.f + dx * dx + dy * dy) *
                                                 (.5f + .5f * std::tanh(8.f * (magnitude.at(x + dx, y + dy) - magnitude.at(x, y))));
                            sum = sum + neighbor * (alignment < 0.f ? -weight : weight);
                        }
                    }
                    const auto tangent = sum.normalized();
                    nextX.at(x, y) = tangent.x;
                    nextY.at(x, y) = tangent.y;
                }
            }
            horizontal.values.swap(nextX.values);
            vertical.values.swap(nextY.values);
        }
    }

    FlowDirection direction(float x, float y) const {
        x = std::clamp(x, 0.f, float(luminance.width - 1));
        y = std::clamp(y, 0.f, float(luminance.height - 1));
        const int ix = int(x), iy = int(y);
        const FlowDirection reference{horizontal.at(ix, iy), vertical.at(ix, iy)};
        FlowDirection sum{};
        for (int dy = 0; dy < 2; ++dy) {
            for (int dx = 0; dx < 2; ++dx) {
                auto tangent = FlowDirection{horizontal.at(ix + dx, iy + dy), vertical.at(ix + dx, iy + dy)};
                if (tangent.dot(reference) < 0.f) tangent = tangent * -1.f;
                const float weight = (dx ? x - ix : 1.f - x + ix) * (dy ? y - iy : 1.f - y + iy);
                sum = sum + tangent * weight;
            }
        }
        return sum.normalized();
    }

    template<typename F>
    float integrate(float x, float y, float radius, F sample) const {
        const int steps = std::clamp(int(std::ceil(radius)), 1, 96);
        const float sigma = std::max(1.f, radius * .4f);
        float sum = sample(x, y), weights = 1.f;
        for (int sign : {-1, 1}) {
            float px = x, py = y;
            auto previous = direction(px, py) * float(sign);
            for (int step = 1; step <= steps; ++step) {
                if ((step & 31) == 0 && EncodingInterruption::currentRequested()) return 0.f;
                auto tangent = direction(px, py);
                if (tangent.dot(previous) < 0.f) tangent = tangent * -1.f;
                auto midpoint = direction(px + tangent.x * .5f, py + tangent.y * .5f);
                if (midpoint.dot(tangent) < 0.f) midpoint = midpoint * -1.f;
                px += midpoint.x;
                py += midpoint.y;
                if (px < 0.f || py < 0.f || px > luminance.width - 1 || py > luminance.height - 1) break;
                const float weight = std::exp(-.5f * step * step / (sigma * sigma));
                sum += sample(px, py) * weight;
                weights += weight;
                previous = midpoint;
            }
        }
        return sum / weights;
    }

private:
    HeightField horizontal;
    HeightField vertical;
};

}
