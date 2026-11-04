#include "PhotoFilter.h"
#include "FlowField.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_flowTextureImpl(
        JNIEnv *env, jobject, jobject input, jfloat length, jfloat detail,
        jfloat contrast, jint seed, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        const FlowField flow(image);
        HeightField texture(flow.luminance.width, flow.luminance.height);
        const float size = float(std::max(texture.width, texture.height));
        const float radius = std::max(1.f, length * size);
        const float frequency = detail * 512.f / size;
        const auto noise = [&](float x, float y) {
            x *= frequency;
            y *= frequency;
            const int ix = int(x), iy = int(y);
            return mix(mix(random(ix, iy, uint32_t(seed)), random(ix + 1, iy, uint32_t(seed)), x - ix),
                       mix(random(ix, iy + 1, uint32_t(seed)), random(ix + 1, iy + 1, uint32_t(seed)), x - ix), y - iy);
        };
        const float gain = contrast * std::sqrt(std::max(1.f, radius * frequency));
        for (int y = 0; y < texture.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < texture.width; ++x) {
                if ((x & 15) == 0 && EncodingInterruption::currentRequested()) return;
                texture.at(x, y) = unit(.5f + (flow.integrate(float(x), float(y), radius, noise) - .5f) * gain);
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb original) {
            return original * (.5f + texture.fromImage(x, y, image));
        });
    });
}
