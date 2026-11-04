#include "PhotoFilter.h"

namespace {

struct PageSample {
    photo::Rgb color{};
    float alpha = 0.f;
};

PageSample samplePage(const photo::Image &image, float x, float y) {
    if (x < 0.f || y < 0.f || x > image.width - 1.f || y > image.height - 1.f) return {};
    const int ix = int(x), iy = int(y);
    const float u = x - ix, v = y - iy;
    PageSample result;
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            const int sx = std::min(ix + col, image.width - 1), sy = std::min(iy + row, image.height - 1);
            const float weight = (col ? u : 1.f - u) * (row ? v : 1.f - v);
            const float alpha = image.pixel(sx, sy)[3] / 255.f * weight;
            result.color = result.color + image.read(sx, sy) * alpha;
            result.alpha += alpha;
        }
    }
    if (result.alpha > 0.f) result.color = result.color * (1.f / result.alpha);
    return result;
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_pageCurlImpl(
        JNIEnv *env, jobject, jobject input, jfloat amount, jfloat radius,
        jfloat angle, jfloat shadow, jint backside, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const float radians = std::clamp(angle, 0.f, 90.f) * photo::pi / 180.f;
        const float nx = std::cos(radians), ny = std::sin(radians);
        const float extent = nx * image.width + ny * image.height;
        const float fold = std::clamp(amount, 0.f, .75f) * extent;
        const float cylinder = std::clamp(radius, .015f, .2f) * std::max(image.width, image.height);
        const float opacity = photo::unit(strength), shade = photo::unit(shadow);
        const photo::Rgb backColor = {((uint32_t(backside) >> 16) & 255) / 255.f,
                                      ((uint32_t(backside) >> 8) & 255) / 255.f,
                                      (uint32_t(backside) & 255) / 255.f};
        const float backAlpha = (uint32_t(backside) >> 24) / 255.f;
        for (int y = 0; y < image.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < image.width; ++x) {
                if ((x & 511) == 0 && EncodingInterruption::currentRequested()) return;
                const auto original = image.read(x, y);
                const float originalAlpha = image.pixel(x, y)[3] / 255.f;
                const float d = nx * (image.width - .5f - x) + ny * (image.height - .5f - y);
                PageSample result = d >= fold || fold <= 0.f ? PageSample{original, originalAlpha} : PageSample{};
                if (fold > 0.f) {
                    // Arc length is measured on the original sheet, then inverse-mapped from a cylinder.
                    const float q = (fold - d) / cylinder;
                    bool curled = false;
                    float surfaceHeight = 0.f;
                    auto surface = [&](float arc, float light, bool back, float height) {
                        const float sourceD = fold - arc;
                        const auto source = samplePage(image, x + nx * (d - sourceD), y + ny * (d - sourceD));
                        if (source.alpha <= 0.f) return false;
                        result = source;
                        if (back) {
                            // A little show-through belongs to the same mirrored sheet, not a flat overlay.
                            result.color = photo::mix(backColor, source.color, .08f) * light;
                            result.alpha *= backAlpha;
                        } else result.color = source.color * light;
                        if (d >= fold && result.alpha < 1.f) {
                            const float underAlpha = originalAlpha * (1.f - result.alpha);
                            const float combinedAlpha = result.alpha + underAlpha;
                            const auto associated = result.color * result.alpha + original * underAlpha;
                            result.color = associated * (combinedAlpha > 0.f ? 1.f / combinedAlpha : 0.f);
                            result.alpha = combinedAlpha;
                        }
                        surfaceHeight = height;
                        return true;
                    };
                    if (q >= 0.f && q <= 1.f) {
                        const float front = std::asin(q), back = photo::pi - front;
                        const float light = .55f + .45f * std::sqrt(std::max(0.f, 1.f - q * q));
                        curled = surface(cylinder * back, light, true, cylinder * (1.f - std::cos(back)));
                        if (!curled) curled = surface(cylinder * front, light, false, cylinder * (1.f - std::cos(front)));
                    } else if (d > fold) {
                        curled = surface(photo::pi * cylinder + d - fold, .94f, true, 2.f * cylinder);
                    }
                    if (!curled && result.alpha > 0.f) {
                        const float distance = std::abs(d - fold);
                        const float shadowWidth = std::max(1.f, cylinder * .65f);
                        const float occlusion = std::exp(-distance * distance / (shadowWidth * shadowWidth));
                        result.color = result.color * (1.f - .65f * shade * occlusion);
                    } else if (curled && surfaceHeight > 0.f) {
                        // Contact shading at the roll is independent of its source texture.
                        result.color = result.color * (1.f - .12f * shade * std::exp(-surfaceHeight / cylinder));
                    }
                }
                const float alpha = photo::mix(originalAlpha, result.alpha, opacity);
                const auto associated = photo::mix(original * originalAlpha, result.color * result.alpha, opacity);
                output.write(x, y, associated * (alpha > 0.f ? 1.f / alpha : 0.f), uint8_t(alpha * 255.f + .5f));
            }
        }
    });
}
