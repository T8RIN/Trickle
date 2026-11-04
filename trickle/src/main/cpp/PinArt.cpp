#include <limits>
#include "PhotoFilter.h"
#include "HeightField.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_pinArtImpl(
        JNIEnv *env, jobject, jobject input, jfloat spacing, jfloat height,
        jfloat angle, jfloat gloss, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        const HeightField relief(image);
        std::array<HeightField, 3> surface = {HeightField(relief.width, relief.height),
                                            HeightField(relief.width, relief.height),
                                            HeightField(relief.width, relief.height)};
        HeightField depth(relief.width, relief.height);
        std::fill(depth.values.begin(), depth.values.end(), -std::numeric_limits<float>::max());
        const float scaleX = float(image.width) / relief.width, scaleY = float(image.height) / relief.height;
        for (int y = 0; y < relief.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < relief.width; ++x) {
                const auto background = image.sample(x * scaleX, y * scaleY) * .16f;
                surface[0].at(x, y) = background.r;
                surface[1].at(x, y) = background.g;
                surface[2].at(x, y) = background.b;
            }
        }
        const float size = float(std::max(relief.width, relief.height));
        const float step = std::max(2.5f, spacing * size / 1024.f);
        const float radius = step * .43f;
        const float sine = std::sin(angle * pi / 180.f), cosine = std::cos(angle * pi / 180.f);
        const float halfY = .4f + sine, halfZ = .75f + cosine;
        const float halfLength = std::sqrt(.45f * .45f + halfY * halfY + halfZ * halfZ);
        for (float cy = step * .5f; cy < relief.height + radius; cy += step) {
            if (EncodingInterruption::currentRequested()) return;
            for (float cx = step * .5f; cx < relief.width + radius; cx += step) {
                if (EncodingInterruption::currentRequested()) return;
                const auto pigment = image.sample(cx * scaleX, cy * scaleY);
                const float elevation = height * size * relief.sample(cx, cy);
                const float topY = cy - elevation * sine;
                const int minX = std::max(0, int(std::floor(cx - radius)));
                const int maxX = std::min(relief.width - 1, int(std::ceil(cx + radius)));
                const int minY = std::max(0, int(std::floor(topY - radius * cosine)));
                const int maxY = std::min(relief.height - 1, int(std::ceil(cy + radius * cosine)));
                for (int y = minY; y <= maxY; ++y) {
                    if ((y & 31) == 0 && EncodingInterruption::currentRequested()) return;
                    for (int x = minX; x <= maxX; ++x) {
                        const float nx = (x + .5f - cx) / radius;
                        if (std::abs(nx) >= 1.f) continue;
                        float distance = -std::numeric_limits<float>::max();
                        Rgb color{};
                        const float front = std::sqrt(1.f - nx * nx);
                        if (sine > 1e-4f) {
                            const float z = (cy + radius * front * cosine - y - .5f) / sine;
                            if (z >= 0.f && z <= elevation) {
                                distance = cy * sine / cosine + radius * front * sine + z * cosine;
                                const float diffuse = std::max(0.f, -.45f * nx + .4f * front);
                                const float reflection = std::max(0.f, (-.45f * nx + halfY * front) / halfLength);
                                color = mix(pigment, gray(.48f), gloss * .4f) * (.18f + .6f * diffuse) +
                                        gray(gloss * .7f * std::pow(reflection, 28.f));
                            }
                        }
                        const float ny = (y + .5f - topY) / (radius * cosine);
                        const float radial = nx * nx + ny * ny;
                        if (radial <= 1.f) {
                            const float topDepth = cy * sine / cosine + radius * ny * sine + elevation * cosine;
                            if (topDepth > distance) {
                                distance = topDepth;
                                const float bevel = smooth(.5f, 1.f, radial);
                                const float normalZ = std::sqrt(std::max(.1f, 1.f - bevel * bevel * radial * .6f));
                                const float diffuse = std::max(0.f, -.45f * nx * bevel + .4f * ny * bevel + .75f * normalZ);
                                const float reflection = std::max(0.f, (-.45f * nx * bevel + halfY * ny * bevel + halfZ * normalZ) /
                                                                        (halfLength * std::sqrt(radial * bevel * bevel + normalZ * normalZ)));
                                color = pigment * (.35f + .65f * diffuse) + gray(gloss * .45f * std::pow(reflection, 40.f));
                            }
                        }
                        if (distance > depth.at(x, y)) {
                            depth.at(x, y) = distance;
                            surface[0].at(x, y) = color.r;
                            surface[1].at(x, y) = color.g;
                            surface[2].at(x, y) = color.b;
                        }
                    }
                }
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb) {
            return Rgb{surface[0].fromImage(x, y, image), surface[1].fromImage(x, y, image), surface[2].fromImage(x, y, image)};
        });
    });
}
