#include "PhotoFilter.h"
#include "HeightField.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_photoelasticStressImpl(
        JNIEnv *env, jobject, jobject input, jfloat load, jfloat smoothing,
        jfloat angle, jfloat ambient, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        HeightField potential(image);
        const int radius = std::max(1, int(smoothing * std::max(potential.width, potential.height) / 1024.f));
        potential.blur(radius);
        potential.blur(radius);
        HeightField difference(potential.width, potential.height);
        HeightField extinction(potential.width, potential.height);
        double energy = 0.;
        for (int y = 0; y < potential.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < potential.width; ++x) {
                const float xx = potential.at(x, y + 1) + potential.at(x, y - 1) - 2.f * potential.at(x, y);
                const float yy = potential.at(x + 1, y) + potential.at(x - 1, y) - 2.f * potential.at(x, y);
                const float xy = -.25f * (potential.at(x + 1, y + 1) - potential.at(x - 1, y + 1) -
                                         potential.at(x + 1, y - 1) + potential.at(x - 1, y - 1));
                const float stress = std::sqrt((xx - yy) * (xx - yy) + 4.f * xy * xy);
                difference.at(x, y) = stress;
                const float direction = .5f * std::atan2(2.f * xy, xx - yy);
                const float transmission = std::sin(2.f * (direction - angle * pi / 180.f));
                extinction.at(x, y) = transmission * transmission;
                energy += stress * stress;
            }
        }
        const float normalization = std::max(.0001f, float(std::sqrt(energy / difference.values.size())));
        std::array<HeightField, 3> transmission = {HeightField(potential.width, potential.height),
                                                 HeightField(potential.width, potential.height),
                                                 HeightField(potential.width, potential.height)};
        constexpr std::array<float, 3> wavelengths = {610.f, 550.f, 460.f};
        for (int y = 0; y < potential.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < potential.width; ++x) {
                const float retardance = 275.f * load * std::min(6.f, difference.at(x, y) / normalization);
                for (int channel = 0; channel < 3; ++channel) {
                    transmission[channel].at(x, y) = extinction.at(x, y) *
                            (.5f - .5f * std::cos(2.f * pi * retardance / wavelengths[channel]));
                }
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb original) {
            const Rgb polarized{transmission[0].fromImage(x, y, image),
                                transmission[1].fromImage(x, y, image),
                                transmission[2].fromImage(x, y, image)};
            return original * ambient + polarized * ((1.f - ambient) * (.5f + .5f * std::sqrt(luma(original))));
        });
    });
}
