#include "PhotoEffects.h"
#include <vector>

namespace photo {
namespace {

Rgb fromColor(uint32_t argb) {
    return {float((argb >> 16u) & 255u) / 255.f,
            float((argb >> 8u) & 255u) / 255.f,
            float(argb & 255u) / 255.f};
}

Rgb multiply(Rgb rgb, const std::array<Rgb, 3> &matrix) {
    return {rgb.r * matrix[0].r + rgb.g * matrix[0].g + rgb.b * matrix[0].b,
            rgb.r * matrix[1].r + rgb.g * matrix[1].g + rgb.b * matrix[1].b,
            rgb.r * matrix[2].r + rgb.g * matrix[2].g + rgb.b * matrix[2].b};
}

Rgb toOklab(Rgb rgb) {
    const auto linear = rgb.map(toLinear);
    const auto lms = multiply(linear, {{{.41222147f, .53633254f, .05144599f},
                                       {.21190350f, .68069955f, .10739696f},
                                       {.08830246f, .28171884f, .62997870f}}}).map([](float v) {
        return std::cbrt(v);
    });
    return multiply(lms, {{{.21045426f, .79361779f, -.00407205f},
                           {1.97799850f, -2.42859221f, .45059371f},
                           {.02590404f, .78277177f, -.80867577f}}});
}

}

void filmGrain(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const float scale = std::max(image.scale(), .001f);
    const float cell = std::max(.65f, p[0] * scale);
    const float footprint = std::min(.45f, .5f / cell);
    render(image, output, amount, [&](int x, int y, Rgb original) {
        const float u = (x + .5f) / cell;
        const float v = (y + .5f) / cell;
        const int ix = int(u), iy = int(v);
        const int adjacentX = fraction(u) < .5f ? -1 : 1;
        const int adjacentY = fraction(v) < .5f ? -1 : 1;
        float coverage = 0.f, colored = 0.f;
        for (const int dy : {0, adjacentY}) {
            for (const int dx : {0, adjacentX}) {
                const int gx = ix + dx, gy = iy + dy;
                const uint32_t bits = hash(gx, gy, seed);
                const float cx = gx + .28f + .44f * (bits & 255u) / 255.f;
                const float cy = gy + .28f + .44f * ((bits >> 8u) & 255u) / 255.f;
                const float radius = .31f + p[2] * .18f * (((bits >> 16u) & 255u) / 127.5f - 1.f);
                const float distance = std::sqrt((u - cx) * (u - cx) + (v - cy) * (v - cy));
                const float grain = 1.f - smooth(radius - footprint, radius + footprint, distance);
                // Approximate the union of grain footprints in the pixel.
                const float visible = (1.f - coverage) * grain;
                coverage += visible;
                colored += visible * (((bits >> 24u) & 255u) / 127.5f - 1.f);
            }
        }
        const float exposure = luma(original);
        const float density = mix(1.f, .25f + .75f * 4.f * exposure * (1.f - exposure), p[3]);
        return original + (gray(coverage - .32f) + Rgb{colored, -.35f * colored, -.7f * colored} * p[4]) *
                          (p[1] * density);
    });
}

void filmNegative(const Image &image, const Output &output, const Params &p, uint32_t baseColor, float amount) {
    const auto base = fromColor(baseColor).map(toLinear);
    const auto floor = base * std::exp(-p[1]);
    const float exposure = std::exp2(p[0]);
    auto develop = [&](float sample, float filmBase, float black) {
        const float transmission = std::max(toLinear(sample), black);
        const float density = std::log(std::max(filmBase, .0001f) / std::max(transmission, .0001f));
        const float signal = unit(density / p[1] * exposure);
        const float contrast = std::pow(signal, p[2]);
        return toSrgb(mix(contrast, contrast * contrast * (3.f - 2.f * contrast), p[3]));
    };
    render(image, output, amount, [&](int, int, Rgb c) {
        return Rgb{develop(c.r, base.r, floor.r), develop(c.g, base.g, floor.g),
                   develop(c.b, base.b, floor.b)};
    });
}

void agxToneMapping(const Image &image, const Output &output, const Params &p, uint32_t, float amount) {
    // Troy Sobotka's AgX matrices for linear sRGB primaries and Benjamin Wrensch's sigmoid fit.
    // https://iolite-engine.com/blog_posts/minimal_agx_implementation
    constexpr std::array<Rgb, 3> inset = {{{.84247906f, .07843360f, .07922375f},
                                          {.04232824f, .87846864f, .07916613f},
                                          {.04237565f, .07843360f, .87914297f}}};
    constexpr std::array<Rgb, 3> outset = {{{1.196879f, -.098021f, -.099029f},
                                           {-.0528969f, 1.151903f, -.0989612f},
                                           {-.0529716f, -.0980435f, 1.151074f}}};
    const float exposure = std::exp2(p[0]);
    const int look = int(p[1]);
    render(image, output, amount, [&](int, int, Rgb original) {
        auto encoded = multiply(original.map(toLinear) * exposure, inset).map([](float linear) {
            const float x = unit((std::log2(std::max(linear, 1e-10f)) + 12.47393f) / 16.5f);
            return (((((15.5f * x - 40.14f) * x + 31.96f) * x - 6.868f) * x +
                     .4298f) * x + .1191f) * x - .00232f;
        });
        float saturation = p[2];
        if (look == 1) {
            encoded = Rgb{encoded.r, encoded.g * .9f, encoded.b * .5f}.map([](float v) {
                return std::pow(unit(v), .8f);
            });
            saturation *= 1.2f;
        } else if (look == 2) {
            encoded = encoded.map([](float v) { return std::pow(unit(v), 1.35f); });
            saturation *= 1.35f;
        }
        encoded = mix(gray(luma(encoded)), encoded, saturation);
        return multiply(encoded, outset).map([](float v) {
            return toSrgb(std::pow(unit(v), 2.2f));
        });
    });
}

void waveletDenoise(const Image &image, const Output &output, const Params &p, uint32_t, float amount) {
    constexpr int tile = 96;
    constexpr int halo = 16;
    constexpr int side = tile + 2 * halo;
    const int levels = int(p[2]);
    std::vector<Rgb> coefficients(side * side);
    std::vector<Rgb> scratch(side);
    auto forward = [&](int offset, int stride, int length) {
        for (int i = 0; i < length / 2; ++i) {
            const auto a = coefficients[offset + stride * (i * 2)];
            const auto b = coefficients[offset + stride * (i * 2 + 1)];
            scratch[i] = (a + b) * .5f;
            scratch[i + length / 2] = (a - b) * .5f;
        }
        for (int i = 0; i < length; ++i) coefficients[offset + stride * i] = scratch[i];
    };
    auto inverse = [&](int offset, int stride, int length) {
        for (int i = 0; i < length / 2; ++i) {
            const auto low = coefficients[offset + stride * i];
            const auto high = coefficients[offset + stride * (i + length / 2)];
            scratch[2 * i] = low + high;
            scratch[2 * i + 1] = low - high;
        }
        for (int i = 0; i < length; ++i) coefficients[offset + stride * i] = scratch[i];
    };
    auto shrink = [](float value, float threshold) {
        return std::copysign(std::max(0.f, std::abs(value) - threshold), value);
    };
    for (int top = 0; top < image.height; top += tile) {
        for (int left = 0; left < image.width; left += tile) {
            if (EncodingInterruption::currentRequested()) return;
            for (int y = 0; y < side; ++y) {
                for (int x = 0; x < side; ++x) {
                    const auto rgb = image.read(left + x - halo, top + y - halo);
                    const float luminance = luma(rgb);
                    coefficients[y * side + x] = {luminance, rgb.r - luminance, rgb.b - luminance};
                }
            }
            for (int level = 0; level < levels; ++level) {
                const int length = side >> level;
                for (int i = 0; i < length; ++i) forward(i * side, 1, length);
                for (int i = 0; i < length; ++i) forward(i, side, length);
                const float noise = p[0] / (1 << level);
                const float chroma = p[1] / (1 << level);
                for (int y = 0; y < length; ++y) {
                    for (int x = y < length / 2 ? length / 2 : 0; x < length; ++x) {
                        auto &c = coefficients[y * side + x];
                        c = {shrink(c.r, noise), shrink(c.g, chroma), shrink(c.b, chroma)};
                    }
                }
            }
            for (int level = levels - 1; level >= 0; --level) {
                const int length = side >> level;
                for (int i = 0; i < length; ++i) inverse(i, side, length);
                for (int i = 0; i < length; ++i) inverse(i * side, 1, length);
            }
            for (int y = top; y < std::min(top + tile, image.height); ++y) {
                for (int x = left; x < std::min(left + tile, image.width); ++x) {
                    const auto c = coefficients[(y - top + halo) * side + x - left + halo];
                    const Rgb rgb{c.r + c.g, c.r - (.2126f * c.g + .0722f * c.b) / .7152f, c.r + c.b};
                    output.write(x, y, mix(image.read(x, y), rgb, amount), image.pixel(x, y)[3]);
                }
            }
        }
    }
}

void daltonization(const Image &image, const Output &output, const Params &p, uint32_t, float amount) {
    constexpr std::array<Rgb, 3> rgbToLms = {{{17.8824f, 43.5161f, 4.11935f},
                                             {3.45565f, 27.1554f, 3.86714f},
                                             {.0299566f, .184309f, 1.46709f}}};
    constexpr std::array<Rgb, 3> lmsToRgb = {{{.08094445f, -.13050441f, .11672107f},
                                             {-.01024853f, .05401933f, -.11361471f},
                                             {-.0003653f, -.00412161f, .6935114f}}};
    const int type = int(p[0]);
    render(image, output, amount, [&](int, int, Rgb original) {
        const auto rgb = original.map(toLinear);
        auto lms = multiply(rgb, rgbToLms);
        if (type == 0) lms.r = 2.02344f * lms.g - 2.52581f * lms.b;
        if (type == 1) lms.g = .494207f * lms.r + 1.24827f * lms.b;
        if (type == 2) lms.b = -.395913f * lms.r + .801109f * lms.g;
        const auto lost = (rgb - multiply(lms, lmsToRgb)) * (p[1] * p[2]);
        auto correction = type == 2 ? Rgb{lost.b, -.5f * lost.b, 0.f}
                                    : Rgb{0.f, .7f * lost.r, lost.r + .7f * lost.g};
        correction = correction - gray(luma(correction) * p[3]);
        return (rgb + correction).map(toSrgb);
    });
}

void paletteRemap(const Image &image, const Output &output, const Params &p, uint32_t count, float) {
    std::array<Rgb, 7> palette{}, lab{};
    for (uint32_t i = 0; i < count; ++i) {
        palette[i] = fromColor(uint32_t(p[i + 1]));
        lab[i] = toOklab(palette[i]);
    }
    constexpr int bayer[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
    render(image, output, 1.f, [&](int x, int y, Rgb original) {
        auto sample = toOklab(original);
        sample.r += ((bayer[y & 3][x & 3] + .5f) / 16.f - .5f) * p[0];
        float best = 10.f;
        uint32_t index = 0;
        for (uint32_t i = 0; i < count; ++i) {
            const auto d = sample - lab[i];
            const float distance = d.r * d.r + d.g * d.g + d.b * d.b;
            if (distance < best) {
                best = distance;
                index = i;
            }
        }
        return palette[index];
    });
}

}
