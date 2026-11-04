#include "PhotoFilter.h"
#include <array>
#include <vector>

namespace {

struct YarnPoint { float x; float y; float z; };

struct YarnSample {
    int column = 0;
    int row = 0;
    float coverage = 0.f;
    float shading = 0.f;
};

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_knittedFabricImpl(
        JNIEnv *env, jobject, jobject input, jfloat stitchSize, jfloat yarnWidth,
        jfloat relief, jfloat twist, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const float cell = std::max(.01f, std::clamp(stitchSize, 6.f, 64.f) * image.scale());
        const float radius = std::clamp(yarnWidth, .08f, .34f);
        const float depth = photo::unit(relief), fibers = photo::unit(twist);
        constexpr int segments = 32;
        std::array<YarnPoint, segments + 1> loop{};
        for (int i = 0; i <= segments; ++i) {
            const float angle = 2.f * photo::pi * i / segments;
            const float cosine = std::cos(angle), sine = std::sin(angle);
            // A continuous yarn doubles back into loops; neighboring periods share their endpoints.
            loop[i] = {float(i) / segments + .24f * sine,
                       .45f + .72f * cosine,
                       .18f * sine};
        }
        const int resolution = std::clamp(int(std::ceil(cell * 3.f)), 64, 256);
        std::vector<YarnSample> tile(size_t(resolution) * resolution);
        const float antialias = std::min(.12f, .65f / cell);
        for (int y = 0; y < resolution; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < resolution; ++x) {
                const float u = (x + .5f) / resolution, v = (y + .5f) / resolution;
                float upperSurface = -10.f;
                YarnSample sample{};
                for (int row = -1; row <= 1; ++row) {
                    for (int col = -1; col <= 1; ++col) {
                        for (int segment = 0; segment < segments; ++segment) {
                            const auto &a = loop[segment], &b = loop[segment + 1];
                            const float dx = b.x - a.x, dy = b.y - a.y;
                            const float px = u - col - a.x, py = v - row - a.y;
                            const float t = photo::unit((px * dx + py * dy) / (dx * dx + dy * dy));
                            const float nx = px - t * dx, ny = py - t * dy;
                            const float distance = std::hypot(nx, ny);
                            const float coverage = 1.f - photo::smooth(radius - antialias, radius + antialias, distance);
                            if (coverage <= 0.f) continue;
                            const float nz = std::sqrt(std::max(0.f, radius * radius - distance * distance));
                            const float surface = photo::mix(a.z, b.z, t) + nz;
                            if (surface <= upperSurface) continue;
                            upperSurface = surface;
                            const float diffuse = std::max(0.f, (-.35f * nx - .45f * ny + .82f * nz) / radius);
                            const float along = 2.f * photo::pi * (segment + t) / segments;
                            const float across = std::atan2(nx * dy - ny * dx,
                                                           nz * std::hypot(dx, dy));
                            const float fiber = 1.f - fibers * .11f * (.5f + .5f * std::sin(34.f * along + 5.f * across));
                            sample = {col, row, coverage, photo::mix(1.f, .38f + .9f * diffuse, depth) * fiber};
                        }
                    }
                }
                tile[size_t(y) * resolution + x] = sample;
            }
        }
        photo::render(image, output, photo::unit(strength), [&](int x, int y, photo::Rgb original) {
            const float u = (x + .5f) / cell, v = (y + .5f) / cell;
            const int column = int(std::floor(u)), row = int(std::floor(v));
            const int tx = std::min(resolution - 1, int(photo::fraction(u) * resolution));
            const int ty = std::min(resolution - 1, int(photo::fraction(v) * resolution));
            const auto &yarn = tile[size_t(ty) * resolution + tx];
            const auto color = image.sample((column + yarn.column + .5f) * cell - .5f,
                                             (row + yarn.row + .5f) * cell - .5f);
            const auto backing = original * photo::mix(.5f, .18f, depth);
            return photo::mix(backing, color * yarn.shading, yarn.coverage);
        });
    });
}
