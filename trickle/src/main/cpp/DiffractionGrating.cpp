#include "PhotoFilter.h"
#include "HeightField.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_diffractionGratingImpl(
        JNIEnv *env, jobject, jobject input, jfloat spacing, jfloat angle,
        jfloat relief, jfloat bandwidth, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        HeightField height(image);
        height.blur(std::max(2, std::max(height.width, height.height) / 128));
        std::array<HeightField, 3> spectrum = {
                HeightField(height.width, height.height),
                HeightField(height.width, height.height),
                HeightField(height.width, height.height)
        };
        constexpr std::array<float, 3> wavelengths = {.610f, .540f, .460f};
        const float vx = std::cos(angle * pi / 180.f), vy = std::sin(angle * pi / 180.f);
        const float size = float(std::max(height.width, height.height));
        const float spread = .09f * bandwidth;
        for (int y = 0; y < height.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < height.width; ++x) {
                const float position = ((x - .5f * height.width) * vx + (y - .5f * height.height) * vy) / size;
                const float slope = (height.dx(x, y) * vx + height.dy(x, y) * vy) * size * .05f * relief;
                const float viewAngle = std::atan(position * 1.6f + slope);
                // Reflection grating: d * (sin(incidence) + sin(view)) = order * wavelength.
                const float path = std::abs(spacing * (std::sin(35.f * pi / 180.f) + std::sin(viewAngle)));
                for (int channel = 0; channel < 3; ++channel) {
                    float energy = 0.f;
                    for (int order = 1; order <= 12; ++order) {
                        const float distance = (path - order * wavelengths[channel]) / (spread * std::sqrt(float(order)));
                        energy += std::exp(-.5f * distance * distance) / std::sqrt(float(order));
                    }
                    spectrum[channel].at(x, y) = unit(energy);
                }
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb original) {
            const float base = .2f + .8f * luma(original);
            return original * .6f + Rgb{spectrum[0].fromImage(x, y, image),
                                       spectrum[1].fromImage(x, y, image),
                                       spectrum[2].fromImage(x, y, image)} * (.6f * base);
        });
    });
}
