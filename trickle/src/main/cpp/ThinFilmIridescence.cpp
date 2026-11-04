#include "PhotoFilter.h"
#include "HeightField.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_thinFilmIridescenceImpl(
        JNIEnv *env, jobject, jobject input, jfloat thickness, jfloat variation,
        jfloat angle, jfloat saturation, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        HeightField height(image);
        height.blur(std::max(1, std::max(height.width, height.height) / 384));
        std::array<HeightField, 3> reflectance = {
                HeightField(height.width, height.height),
                HeightField(height.width, height.height),
                HeightField(height.width, height.height)
        };
        constexpr float index = 1.46f;
        constexpr std::array<float, 3> wavelengths = {610.f, 550.f, 460.f};
        const float viewX = std::sin(angle * pi / 180.f);
        const float viewZ = std::cos(angle * pi / 180.f);
        const float size = float(std::max(height.width, height.height));
        for (int y = 0; y < height.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < height.width; ++x) {
                const float gx = height.dx(x, y) * size * .03f;
                const float gy = height.dy(x, y) * size * .03f;
                const float cosine = unit(std::abs(viewZ - viewX * gx) / std::sqrt(1.f + gx * gx + gy * gy));
                const float refractedCosine = std::sqrt(1.f - (1.f - cosine * cosine) / (index * index));
                const float film = std::max(0.f, thickness + variation * (height.at(x, y) - .5f));
                const float opticalPath = 2.f * index * film * refractedCosine;
                for (int channel = 0; channel < 3; ++channel) {
                    reflectance[channel].at(x, y) = .5f - .5f * std::cos(2.f * pi * opticalPath / wavelengths[channel]);
                }
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb original) {
            const Rgb spectrum{reflectance[0].fromImage(x, y, image),
                               reflectance[1].fromImage(x, y, image),
                               reflectance[2].fromImage(x, y, image)};
            const auto tint = mix(gray(luma(spectrum)), spectrum, saturation);
            return Rgb{toSrgb(toLinear(original.r) * (.45f + 1.1f * tint.r)),
                       toSrgb(toLinear(original.g) * (.45f + 1.1f * tint.g)),
                       toSrgb(toLinear(original.b) * (.45f + 1.1f * tint.b))};
        });
    });
}
