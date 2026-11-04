#include "PhotoFilter.h"
#include "HeightField.h"

namespace {

struct AlphaSample {
    photo::Rgb color{};
    float alpha = 0.f;

    photo::Rgb straight(photo::Rgb fallback) const {
        return alpha > 1e-6f ? color * (1.f / alpha) : fallback;
    }
};

AlphaSample samplePremultiplied(const photo::Image &image, float x, float y) {
    x = std::clamp(x, 0.f, float(image.width - 1));
    y = std::clamp(y, 0.f, float(image.height - 1));
    const int ix = int(x), iy = int(y);
    AlphaSample result;
    for (int dy = 0; dy < 2; ++dy) {
        for (int dx = 0; dx < 2; ++dx) {
            const int px = std::min(ix + dx, image.width - 1);
            const int py = std::min(iy + dy, image.height - 1);
            const float weight = (dx ? x - ix : 1.f - x + ix) * (dy ? y - iy : 1.f - y + iy);
            const float alpha = image.pixel(px, py)[3] * (weight / 255.f);
            result.color = result.color + image.read(px, py) * alpha;
            result.alpha += alpha;
        }
    }
    return result;
}

struct Droplet {
    float height = 0.f;
    float x = 0.f;
    float y = 0.f;
    float highlight = 0.f;
    float rim = 0.f;
    float coverage = 0.f;
};

void sampleDroplets(Droplet &result, float x, float y, float spacing, float coverage, float refraction, uint32_t seed) {
    using namespace photo;
    const int ix = int(std::floor(x / spacing)), iy = int(std::floor(y / spacing));
    for (int cy = iy - 1; cy <= iy + 1; ++cy) {
        for (int cx = ix - 1; cx <= ix + 1; ++cx) {
            if (random(cx, cy, seed ^ 0x72b45u) >= coverage) continue;
            const float centerX = (cx + .35f + .3f * random(cx, cy, seed)) * spacing;
            const float centerY = (cy + .35f + .3f * random(cx, cy, seed ^ 0x37ac5u)) * spacing;
            const float radius = spacing * (.18f + .17f * random(cx, cy, seed ^ 0xaf487u));
            const float elongation = 1.f + .3f * random(cx, cy, seed ^ 0x9657u);
            const float nx = (x - centerX) / radius, ny = (y - centerY) / (radius * elongation);
            const float radial = nx * nx + ny * ny;
            if (radial >= 1.f) continue;
            const float nz = std::sqrt(1.f - radial);
            const float height = radius * nz;
            if (height <= result.height) continue;
            constexpr float eta = 1.f / 1.333f;
            const float normalScale = eta * nz - std::sqrt(1.f - eta * eta * radial);
            const float tz = -eta + normalScale * nz;
            const float distance = radius * refraction * 2.5f;
            const float highlight = std::max(0.f, -.22f * nx - .32f * ny + .921f * nz);
            result.height = height;
            result.x = distance * normalScale * nx / -tz;
            result.y = distance * normalScale * ny / -tz;
            result.highlight = .85f * std::pow(highlight, 42.f);
            result.rim = std::pow(1.f - nz, 3.f);
            result.coverage = 1.f - smooth(1.f - std::min(.5f, 1.5f / radius), 1.f, radial);
        }
    }
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_condensationImpl(
        JNIEnv *env, jobject, jobject input, jfloat size, jfloat coverage,
        jfloat refraction, jfloat haze, jint seed, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        const HeightField dimensions(image);
        std::array<HeightField, 4> fog = {HeightField(dimensions.width, dimensions.height),
                                        HeightField(dimensions.width, dimensions.height),
                                        HeightField(dimensions.width, dimensions.height),
                                        HeightField(dimensions.width, dimensions.height)};
        HeightField offsetX(dimensions.width, dimensions.height), offsetY(dimensions.width, dimensions.height);
        HeightField highlights(dimensions.width, dimensions.height), rims(dimensions.width, dimensions.height);
        HeightField masks(dimensions.width, dimensions.height);
        const float scaleX = float(image.width) / dimensions.width, scaleY = float(image.height) / dimensions.height;
        const float spacing = std::max(3.f, size * std::max(dimensions.width, dimensions.height) / 1024.f);
        for (int y = 0; y < dimensions.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < dimensions.width; ++x) {
                if ((x & 63) == 0 && EncodingInterruption::currentRequested()) return;
                const auto source = samplePremultiplied(image, x * scaleX, y * scaleY);
                fog[0].at(x, y) = source.color.r;
                fog[1].at(x, y) = source.color.g;
                fog[2].at(x, y) = source.color.b;
                fog[3].at(x, y) = source.alpha;
                Droplet droplet;
                sampleDroplets(droplet, float(x), float(y), spacing, coverage, refraction, uint32_t(seed));
                sampleDroplets(droplet, float(x), float(y), spacing * .32f, coverage * .75f, refraction, uint32_t(seed) ^ 0x95ab3u);
                offsetX.at(x, y) = droplet.x;
                offsetY.at(x, y) = droplet.y;
                highlights.at(x, y) = droplet.highlight;
                rims.at(x, y) = droplet.rim;
                masks.at(x, y) = droplet.coverage;
            }
        }
        for (auto &channel : fog) channel.blur(std::max(1, int(spacing * .3f)));
        render(image, output, strength, [&](int x, int y, Rgb original) {
            const AlphaSample blurred{{fog[0].fromImage(x, y, image), fog[1].fromImage(x, y, image), fog[2].fromImage(x, y, image)},
                                       fog[3].fromImage(x, y, image)};
            const auto mist = mix(original, mix(blurred.straight(original), gray(.9f), .35f), haze);
            const float dx = offsetX.fromImage(x, y, image) * scaleX;
            const float dy = offsetY.fromImage(x, y, image) * scaleY;
            const float rim = rims.fromImage(x, y, image);
            const auto refracted = samplePremultiplied(image, x + dx, y + dy).straight(original) * (1.f - .45f * rim) +
                                   gray(highlights.fromImage(x, y, image) + .12f * rim);
            return mix(mist, refracted, masks.fromImage(x, y, image));
        });
    });
}
