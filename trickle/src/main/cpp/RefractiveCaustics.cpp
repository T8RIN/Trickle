#include "PhotoFilter.h"
#include "HeightField.h"

namespace {

void splat(photo::HeightField &field, float x, float y) {
    if (x <= -1.f || y <= -1.f || x >= field.width || y >= field.height) return;
    const int ix = int(std::floor(x)), iy = int(std::floor(y));
    const float fx = x - ix, fy = y - iy;
    for (int dy = 0; dy < 2; ++dy) {
        for (int dx = 0; dx < 2; ++dx) {
            if (ix + dx < 0 || iy + dy < 0 || ix + dx >= field.width || iy + dy >= field.height) continue;
            field.at(ix + dx, iy + dy) += (dx ? fx : 1.f - fx) * (dy ? fy : 1.f - fy);
        }
    }
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_refractiveCausticsImpl(
        JNIEnv *env, jobject, jobject input, jfloat scale, jfloat refraction,
        jfloat dispersion, jfloat focus, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        HeightField height(image);
        const float size = float(std::max(height.width, height.height));
        const int smoothing = std::max(1, int(.008f * size / scale));
        height.blur(smoothing);
        height.blur(smoothing);
        std::array<HeightField, 3> light = {
                HeightField(height.width, height.height),
                HeightField(height.width, height.height),
                HeightField(height.width, height.height)
        };
        const float distance = refraction * size;
        for (int y = 0; y < height.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < height.width; ++x) {
                const float gx = height.dx(x, y) * size * .08f;
                const float gy = height.dy(x, y) * size * .08f;
                const float nz = 1.f / std::sqrt(1.f + gx * gx + gy * gy);
                const float nx = -gx * nz, ny = -gy * nz;
                for (int channel = 0; channel < 3; ++channel) {
                    const float eta = 1.f / (1.33f + (channel - 1) * .06f * dispersion);
                    const float normalScale = eta * nz - std::sqrt(1.f - eta * eta * (1.f - nz * nz));
                    const float tx = normalScale * nx, ty = normalScale * ny;
                    const float tz = -eta + normalScale * nz;
                    splat(light[channel], x + distance * tx / -tz, y + distance * ty / -tz);
                }
            }
        }
        for (auto &channel : light) channel.blur(1);
        render(image, output, strength, [&](int x, int y, Rgb original) {
            const auto illumination = [&](int channel) {
                return std::pow(std::max(.02f, .25f + .75f * light[channel].fromImage(x, y, image)), focus);
            };
            return Rgb{toSrgb(toLinear(original.r) * illumination(0)),
                       toSrgb(toLinear(original.g) * illumination(1)),
                       toSrgb(toLinear(original.b) * illumination(2))};
        });
    });
}
