#include "PhotoFilter.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_localBinaryPatternImpl(
        JNIEnv *env, jobject, jobject input, jfloat radius, jfloat tolerance, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        const float distance = std::max(.5f, radius * image.scale());
        std::array<float, 8> dx{}, dy{};
        for (int i = 0; i < 8; ++i) {
            dx[i] = std::cos(i * pi / 4.f) * distance;
            dy[i] = std::sin(i * pi / 4.f) * distance;
        }
        render(image, output, strength, [&](int x, int y, Rgb color) {
            const float center = luma(color) + tolerance;
            unsigned code = 0;
            for (int i = 0; i < 8; ++i) {
                if (luma(image.sample(x + dx[i], y + dy[i])) >= center) code |= 1u << i;
            }
            return gray(code / 255.f);
        });
    });
}
