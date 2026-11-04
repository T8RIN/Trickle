#include "PhotoFilter.h"
#include "HeightField.h"
#include "MaterialCells.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_marquetryImpl(
        JNIEnv *env, jobject, jobject input, jfloat size, jfloat grain,
        jfloat bevel, jint seed, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        const HeightField height(image);
        std::array<HeightField, 3> wood = {HeightField(height.width, height.height),
                                         HeightField(height.width, height.height),
                                         HeightField(height.width, height.height)};
        const float cellSize = std::max(3.f, size * std::max(height.width, height.height) / 1024.f);
        const float scaleX = float(image.width) / height.width, scaleY = float(image.height) / height.height;
        for (int y = 0; y < height.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < height.width; ++x) {
                if ((x & 63) == 0 && EncodingInterruption::currentRequested()) return;
                const auto cell = materialCell(x / cellSize, y / cellSize, uint32_t(seed));
                const float cx = cell.centerX * cellSize, cy = cell.centerY * cellSize;
                const float orientation = pi * random(cell.x, cell.y, uint32_t(seed) ^ 0xab35u);
                const float vx = std::cos(orientation), vy = std::sin(orientation);
                const float u = (x - cx) / cellSize, v = (y - cy) / cellSize;
                const float along = u * vx + v * vy, across = -u * vy + v * vx;
                const float knot = .7f + random(cell.x, cell.y, uint32_t(seed) ^ 0x34ac5u);
                const float rings = across * 10.f + 1.3f * std::sin(along * 2.6f) +
                                    .6f * std::log(.08f + (along - knot) * (along - knot) + across * across * 3.f);
                const float earlyWood = .5f + .5f * std::sin(2.f * pi * rings);
                const float lateWood = std::pow(earlyWood, 7.f);
                const float fibers = .5f + .5f * std::sin(2.f * pi * (across * 61.f + .4f * std::sin(along * 3.f)));
                const auto source = image.sample(cx * scaleX, cy * scaleY);
                const float tone = unit(luma(source));
                const auto timber = mix(Rgb{.12f, .055f, .022f}, Rgb{.91f, .76f, .48f}, std::pow(tone, .65f));
                const auto stain = mix(timber, source, .28f);
                const float texture = 1.f - grain * (.23f * lateWood + .07f * fibers);
                const float edge = std::exp(-cell.edge * cellSize / std::max(.7f, bevel * 1.8f));
                const float lighting = 1.f + bevel * edge * (.3f * (-cell.normalX - cell.normalY) - .16f);
                const float seam = 1.f - .6f * (1.f - smooth(0.f, .55f / cellSize, cell.edge));
                const auto color = stain * (texture * lighting * seam);
                wood[0].at(x, y) = color.r;
                wood[1].at(x, y) = color.g;
                wood[2].at(x, y) = color.b;
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb) {
            return Rgb{wood[0].fromImage(x, y, image), wood[1].fromImage(x, y, image), wood[2].fromImage(x, y, image)};
        });
    });
}
