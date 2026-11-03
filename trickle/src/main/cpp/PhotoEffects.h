#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include "EncodingInterruption.h"

namespace photo {

constexpr float pi = 3.14159265358979323846f;

inline float unit(float value) {
    return std::clamp(value, 0.f, 1.f);
}

inline float mix(float a, float b, float amount) {
    return a + (b - a) * amount;
}

inline float smooth(float low, float high, float value) {
    const float t = unit((value - low) / (high - low));
    return t * t * (3.f - 2.f * t);
}

inline float fraction(float value) {
    return value - std::floor(value);
}

struct Rgb {
    float r;
    float g;
    float b;

    Rgb operator+(Rgb v) const { return {r + v.r, g + v.g, b + v.b}; }
    Rgb operator-(Rgb v) const { return {r - v.r, g - v.g, b - v.b}; }
    Rgb operator*(float v) const { return {r * v, g * v, b * v}; }

    template<typename F>
    Rgb map(F f) const { return {f(r), f(g), f(b)}; }
};

inline Rgb gray(float value) {
    return {value, value, value};
}

inline Rgb mix(Rgb a, Rgb b, float amount) {
    return a + (b - a) * amount;
}

inline float luma(Rgb rgb) {
    return .2126f * rgb.r + .7152f * rgb.g + .0722f * rgb.b;
}

inline float toLinear(float v) {
    return v <= .04045f ? v / 12.92f : std::pow((v + .055f) / 1.055f, 2.4f);
}

inline float toSrgb(float v) {
    return v <= .0031308f ? v * 12.92f : 1.055f * std::pow(v, 1.f / 2.4f) - .055f;
}

inline uint32_t hash(int x, int y, uint32_t seed) {
    uint32_t v = seed ^ (uint32_t(x) * 0x9e3779b1u) ^ (uint32_t(y) * 0x85ebca77u);
    v = (v ^ (v >> 16u)) * 0x7feb352du;
    v = (v ^ (v >> 15u)) * 0x846ca68bu;
    return v ^ (v >> 16u);
}

inline float random(int x, int y, uint32_t seed) {
    return (hash(x, y, seed) >> 8u) * (1.f / 16777216.f);
}

struct Image {
    int width;
    int height;
    size_t stride;
    const uint8_t *pixels;
    bool premultiplied;

    float scale() const { return std::max(width, height) / 1024.f; }

    const uint8_t *pixel(int x, int y) const {
        return pixels + size_t(y) * stride + size_t(x) * 4;
    }

    Rgb read(int x, int y) const {
        const auto *p = pixel(std::clamp(x, 0, width - 1), std::clamp(y, 0, height - 1));
        if (!p[3]) return {};
        const float scale = 1.f / (premultiplied ? p[3] : 255.f);
        return {unit(p[0] * scale), unit(p[1] * scale), unit(p[2] * scale)};
    }

    Rgb sample(float x, float y) const {
        x = std::clamp(x, 0.f, float(width - 1));
        y = std::clamp(y, 0.f, float(height - 1));
        const int x0 = int(x);
        const int y0 = int(y);
        return mix(mix(read(x0, y0), read(x0 + 1, y0), x - x0),
                   mix(read(x0, y0 + 1), read(x0 + 1, y0 + 1), x - x0), y - y0);
    }
};

struct Output {
    size_t stride;
    uint8_t *pixels;

    void write(int x, int y, Rgb rgb, uint8_t alpha) const {
        auto *p = pixels + size_t(y) * stride + size_t(x) * 4;
        p[0] = uint8_t(unit(rgb.r) * alpha + .5f);
        p[1] = uint8_t(unit(rgb.g) * alpha + .5f);
        p[2] = uint8_t(unit(rgb.b) * alpha + .5f);
        p[3] = alpha;
    }
};

template<typename F>
void render(const Image &image, const Output &output, float amount, F effect) {
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            if ((x & 2047) == 0 && EncodingInterruption::currentRequested()) return;
            const auto original = image.read(x, y);
            output.write(x, y, mix(original, effect(x, y, original), amount), image.pixel(x, y)[3]);
        }
    }
}

using Params = std::array<float, 8>;
using Filter = void (*)(const Image &, const Output &, const Params &, uint32_t, float);

void palComposite(const Image &, const Output &, const Params &, uint32_t, float);
void secam(const Image &, const Output &, const Params &, uint32_t, float);
void rfReception(const Image &, const Output &, const Params &, uint32_t, float);
void sstvReception(const Image &, const Output &, const Params &, uint32_t, float);
void lcdSubpixels(const Image &, const Output &, const Params &, uint32_t, float);
void filmGrain(const Image &, const Output &, const Params &, uint32_t, float);
void filmNegative(const Image &, const Output &, const Params &, uint32_t, float);
void agxToneMapping(const Image &, const Output &, const Params &, uint32_t, float);
void waveletDenoise(const Image &, const Output &, const Params &, uint32_t, float);
void daltonization(const Image &, const Output &, const Params &, uint32_t, float);
void retroHardware(const Image &, const Output &, const Params &, uint32_t, float);
void risograph(const Image &, const Output &, const Params &, uint32_t, float);
void cmykPress(const Image &, const Output &, const Params &, uint32_t, float);
void thermalReceipt(const Image &, const Output &, const Params &, uint32_t, float);
void crossStitch(const Image &, const Output &, const Params &, uint32_t, float);
void autostereogram(const Image &, const Output &, const Params &, uint32_t, float);
void paletteRemap(const Image &, const Output &, const Params &, uint32_t, float);

}
