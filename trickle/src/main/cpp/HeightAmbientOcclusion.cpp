#include "PhotoFilter.h"
#include "HeightField.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_heightAmbientOcclusionImpl(
        JNIEnv *env, jobject, jobject input, jfloat radius, jfloat depth, jint directions, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        HeightField height(image);
        height.blur(1);
        HeightField occlusion(height.width, height.height);
        const float reach = std::max(1.f, radius * std::max(height.width, height.height));
        const float verticalScale = reach * depth;
        const int steps = std::clamp(int(std::ceil(reach)), 2, 24);
        for (int y = 0; y < height.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < height.width; ++x) {
                if ((x & 31) == 0 && EncodingInterruption::currentRequested()) return;
                float blocked = 0.f;
                for (int direction = 0; direction < directions; ++direction) {
                    const float angle = 2.f * pi * (direction + .5f) / directions;
                    const float vx = std::cos(angle), vy = std::sin(angle);
                    const float tangent = verticalScale * (height.dx(x, y) * vx + height.dy(x, y) * vy);
                    float horizon = std::max(0.f, tangent);
                    for (int step = 1; step <= steps; ++step) {
                        const float distance = 1.f + (reach - 1.f) * step * step / (steps * steps);
                        const float px = x + vx * distance, py = y + vy * distance;
                        if (px < 0.f || px > height.width - 1 || py < 0.f || py > height.height - 1) break;
                        const float slope = (height.sample(px, py) - height.at(x, y)) * verticalScale / distance;
                        horizon = std::max(horizon, slope * (1.f - .5f * distance / reach));
                    }
                    const float baseline = std::max(0.f, tangent);
                    blocked += horizon / std::sqrt(1.f + horizon * horizon) -
                               baseline / std::sqrt(1.f + baseline * baseline);
                }
                occlusion.at(x, y) = unit(1.f - .85f * blocked / directions);
            }
        }
        render(image, output, strength, [&](int x, int y, Rgb original) {
            return original * occlusion.fromImage(x, y, image);
        });
    });
}
