#include "PhotoFilter.h"

namespace {

struct FlowVector { float x; float y; };

FlowVector noiseGradient(float x, float y, uint32_t seed) {
    const int ix = int(std::floor(x)), iy = int(std::floor(y));
    const float u = photo::fraction(x), v = photo::fraction(y);
    const float sx = u * u * (3.f - 2.f * u), sy = v * v * (3.f - 2.f * v);
    const float dx = 6.f * u * (1.f - u), dy = 6.f * v * (1.f - v);
    const float a = photo::random(ix, iy, seed), b = photo::random(ix + 1, iy, seed);
    const float c = photo::random(ix, iy + 1, seed), d = photo::random(ix + 1, iy + 1, seed);
    return {photo::mix(b - a, d - c, sy) * dx, photo::mix(c - a, d - b, sx) * dy};
}

FlowVector velocity(float x, float y, uint32_t seed) {
    FlowVector flow{};
    float frequency = 1.f, amplitude = 1.f;
    for (int octave = 0; octave < 3; ++octave) {
        const auto gradient = noiseGradient(x * frequency, y * frequency, seed + uint32_t(octave) * 977u);
        // Curl of a scalar stream function: rotating its gradient preserves zero divergence.
        flow.x += amplitude * gradient.y;
        flow.y -= amplitude * gradient.x;
        frequency *= 2.f;
        amplitude *= .5f;
    }
    return flow;
}

float reflected(float coordinate, int size) {
    if (size <= 1) return 0.f;
    const float period = 2.f * (size - 1);
    const float value = coordinate - std::floor(coordinate / period) * period;
    return std::min(value, period - value);
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_curlAdvectionImpl(
        JNIEnv *env, jobject, jobject input, jfloat scale, jfloat distance, jint steps,
        jint seed, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const float frequency = std::clamp(scale, .5f, 12.f) / std::max(image.width, image.height);
        const int iterations = std::clamp(int(steps), 4, 48);
        const float delta = std::clamp(distance, 0.f, 160.f) * image.scale() / iterations;
        const float amount = photo::unit(strength);
        for (int y = 0; y < image.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < image.width; ++x) {
                if ((x & 63) == 0 && EncodingInterruption::currentRequested()) return;
                float px = x, py = y;
                for (int step = 0; step < iterations; ++step) {
                    const auto first = velocity(px * frequency, py * frequency, uint32_t(seed));
                    const auto middle = velocity((px - .5f * delta * first.x) * frequency,
                                                 (py - .5f * delta * first.y) * frequency, uint32_t(seed));
                    px -= delta * middle.x;
                    py -= delta * middle.y;
                }
                px = reflected(px, image.width);
                py = reflected(py, image.height);
                const int ix = int(px), iy = int(py);
                const float u = px - ix, v = py - iy;
                photo::Rgb warped{};
                float alpha = 0.f;
                for (int row = 0; row < 2; ++row) {
                    for (int col = 0; col < 2; ++col) {
                        const int sx = std::min(ix + col, image.width - 1);
                        const int sy = std::min(iy + row, image.height - 1);
                        const float weight = (col ? u : 1.f - u) * (row ? v : 1.f - v);
                        const float coverage = image.pixel(sx, sy)[3] / 255.f * weight;
                        warped = warped + image.read(sx, sy) * coverage;
                        alpha += coverage;
                    }
                }
                const float originalAlpha = image.pixel(x, y)[3] / 255.f;
                const float resultAlpha = photo::mix(originalAlpha, alpha, amount);
                const auto associated = photo::mix(image.read(x, y) * originalAlpha, warped, amount);
                output.write(x, y, associated * (resultAlpha > 0.f ? 1.f / resultAlpha : 0.f),
                             uint8_t(resultAlpha * 255.f + .5f));
            }
        }
    });
}
