#include "PhotoFilter.h"
#include "HeightField.h"
#include "MaterialCells.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_ceramicGlazeImpl(
        JNIEnv *env, jobject, jobject input, jfloat size, jfloat pooling,
        jfloat gloss, jfloat crackle, jint seed, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        HeightField height(image);
        height.blur(2);
        HeightField absorption(height.width, height.height);
        HeightField reflection(height.width, height.height);
        HeightField seams(height.width, height.height);
        const float cellSize = std::max(3.f, size * std::max(height.width, height.height) / 1024.f);
        for (int y = 0; y < height.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < height.width; ++x) {
                if ((x & 63) == 0 && EncodingInterruption::currentRequested()) return;
                const float px = x / cellSize, py = y / cellSize;
                const auto cell = materialCell(px + .06f * std::sin(py * 2.7f),
                                               py + .06f * std::sin(px * 3.1f), uint32_t(seed));
                const float cavity = std::exp(-cell.edge * 12.f);
                const float thickness = .7f + pooling * (1.1f * cavity + .45f * (1.f - height.at(x, y)));
                absorption.at(x, y) = thickness;
                seams.at(x, y) = crackle * (1.f - smooth(.01f, .01f + .85f / cellSize, cell.edge));
                const float nx = -cell.normalX * cavity * .9f - height.dx(x, y) * 1.2f;
                const float ny = -cell.normalY * cavity * .9f - height.dy(x, y) * 1.2f;
                const float nz = 1.f / std::sqrt(1.f + nx * nx + ny * ny);
                const float highlight = std::max(0.f, (-.28f * nx - .38f * ny + .882f) * nz);
                reflection.at(x, y) = gloss * (.045f + .7f * std::pow(highlight, 44.f));
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb original) {
            const float depth = absorption.fromImage(x, y, image);
            const float seam = seams.fromImage(x, y, image);
            const float light = reflection.fromImage(x, y, image);
            const auto pigment = original.map([&](float value) {
                return toSrgb(std::pow(std::max(.005f, toLinear(value)), depth));
            });
            const auto glazed = mix(pigment, Rgb{.12f, .105f, .075f}, seam * .78f);
            return glazed * (1.f - light * .35f) + Rgb{1.f, .98f, .91f} * light;
        });
    });
}
