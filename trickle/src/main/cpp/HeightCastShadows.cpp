#include "PhotoFilter.h"
#include "HeightField.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_heightCastShadowsImpl(
        JNIEnv *env, jobject, jobject input, jfloat azimuth, jfloat elevation,
        jfloat depth, jfloat softness, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        HeightField height(image);
        height.blur(1);
        HeightField visibility(height.width, height.height);
        const float size = float(std::max(height.width, height.height));
        const float verticalScale = depth * size;
        const float vx = std::cos(azimuth * pi / 180.f);
        const float vy = -std::sin(azimuth * pi / 180.f);
        const float lightSlope = std::tan(elevation * pi / 180.f);
        const float penumbra = std::max(.002f, std::min(softness * .6f, lightSlope * .9f));
        const float reach = std::min(size, verticalScale / std::max(.01f, lightSlope - penumbra) + 2.f);
        const int steps = std::clamp(int(std::ceil(reach)), 2, 96);
        for (int y = 0; y < height.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < height.width; ++x) {
                if ((x & 31) == 0 && EncodingInterruption::currentRequested()) return;
                float horizon = 0.f;
                for (int step = 1; step <= steps; ++step) {
                    const float t = float(step) / steps;
                    const float distance = 1.f + (reach - 1.f) * t * t;
                    const float px = x + vx * distance, py = y + vy * distance;
                    if (px < 0.f || px > height.width - 1 || py < 0.f || py > height.height - 1) break;
                    horizon = std::max(horizon, (height.sample(px, py) - height.at(x, y)) * verticalScale / distance);
                }
                visibility.at(x, y) = 1.f - .8f * smooth(lightSlope - penumbra, lightSlope + penumbra, horizon);
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb original) {
            return original * visibility.fromImage(x, y, image);
        });
    });
}
