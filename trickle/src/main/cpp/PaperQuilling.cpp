#include "PhotoFilter.h"
#include <vector>

namespace {

struct PaperCoil {
    photo::Rgb color;
    float cosine;
    float sine;
    float aspect;
};

struct CoilDistance { float distance; float normalX; float normalY; };

CoilDistance spiralDistance(float x, float y, const PaperCoil &coil, float turns) {
    const float u = (coil.cosine * x + coil.sine * y) / coil.aspect;
    const float v = -coil.sine * x + coil.cosine * y;
    const float radius = std::max(.0001f, std::hypot(u, v));
    float angle = std::atan2(v, u);
    if (angle < 0.f) angle += 2.f * photo::pi;
    const float pitch = .46f / (2.f * photo::pi * turns);
    const float revolution = std::round((radius / pitch - angle) / (2.f * photo::pi));
    float best = 10.f, nx = 0.f, ny = 0.f;
    for (int candidate = -1; candidate <= 1; ++candidate) {
        const float t = std::clamp(angle + 2.f * photo::pi * (revolution + candidate), 0.f, 2.f * photo::pi * turns);
        const float pathRadius = pitch * t;
        const float px = pathRadius * std::cos(t), py = pathRadius * std::sin(t);
        const float dx = (u - px) * coil.aspect, dy = v - py;
        const float distance = std::hypot(dx, dy);
        if (distance >= best) continue;
        best = distance;
        const float inverse = 1.f / std::max(.0001f, distance);
        nx = (coil.cosine * dx - coil.sine * dy) * inverse;
        ny = (coil.sine * dx + coil.cosine * dy) * inverse;
    }
    return {best, nx, ny};
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_paperQuillingImpl(
        JNIEnv *env, jobject, jobject input, jfloat cellSize, jfloat turns, jfloat stripWidth,
        jfloat relief, jint background, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const float cell = std::max(.01f, std::clamp(cellSize, 12.f, 96.f) * image.scale());
        const int columns = std::max(1, int(std::ceil(image.width / cell)));
        const int rows = std::max(1, int(std::ceil(image.height / cell)));
        const float winding = std::clamp(turns, 1.f, 8.f);
        const float radius = .5f * std::clamp(stripWidth, .025f, .18f);
        const float wallHeight = .12f * photo::unit(relief);
        const float antialias = std::min(.1f, .65f / cell);
        const photo::Rgb backgroundColor = {((uint32_t(background) >> 16) & 255) / 255.f,
                                            ((uint32_t(background) >> 8) & 255) / 255.f,
                                            (uint32_t(background) & 255) / 255.f};
        std::vector<PaperCoil> coils(size_t(columns) * rows);
        for (int y = 0; y < rows; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < columns; ++x) {
                const float px = (x + .5f) * cell - .5f, py = (y + .5f) * cell - .5f;
                const float dx = photo::luma(image.sample(px + cell * .35f, py)) - photo::luma(image.sample(px - cell * .35f, py));
                const float dy = photo::luma(image.sample(px, py + cell * .35f)) - photo::luma(image.sample(px, py - cell * .35f));
                const float angle = std::atan2(dy, dx) + photo::pi * .5f;
                coils[size_t(y) * columns + x] = {image.sample(px, py), std::cos(angle), std::sin(angle),
                                                 .72f + .22f * (1.f - photo::unit(std::hypot(dx, dy)))};
            }
        }
        photo::render(image, output, photo::unit(strength), [&](int x, int y, photo::Rgb) {
            const float u = (x + .5f) / cell, v = (y + .5f) / cell;
            const int cx = int(u), cy = int(v);
            photo::Rgb result = backgroundColor;
            for (int row = std::max(0, cy - 1); row <= std::min(rows - 1, cy + 1); ++row) {
                for (int col = std::max(0, cx - 1); col <= std::min(columns - 1, cx + 1); ++col) {
                    const float px = u - col - .5f, py = v - row - .5f;
                    if (std::abs(px) > .65f || std::abs(py) > .7f) continue;
                    const auto &coil = coils[size_t(row) * columns + col];
                    const auto bottom = spiralDistance(px, py, coil, winding);
                    const auto middle = spiralDistance(px + wallHeight * .35f, py + wallHeight * .5f, coil, winding);
                    const auto top = spiralDistance(px + wallHeight * .7f, py + wallHeight, coil, winding);
                    const auto shadow = spiralDistance(px - wallHeight * .65f, py - wallHeight * .6f, coil, winding);
                    const float shadowMask = 1.f - photo::smooth(radius, radius + antialias + wallHeight * .35f, shadow.distance);
                    result = result * (1.f - .25f * photo::unit(relief) * shadowMask);
                    const float sideDistance = std::min({bottom.distance, middle.distance, top.distance});
                    const float sideMask = photo::smooth(-antialias, antialias, sideDistance + radius) -
                                           photo::smooth(-antialias, antialias, sideDistance - radius);
                    const float topMask = photo::smooth(-antialias, antialias, top.distance + radius) -
                                          photo::smooth(-antialias, antialias, top.distance - radius);
                    const float light = .5f + .2f * (-middle.normalX - middle.normalY) * .70710678f;
                    const auto wallColor = coil.color * light;
                    const auto edgeColor = photo::mix(coil.color, backgroundColor, .18f);
                    result = photo::mix(result, wallColor, sideMask);
                    result = photo::mix(result, edgeColor, topMask);
                }
            }
            return result;
        });
    });
}
