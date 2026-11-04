#include "PhotoFilter.h"
#include <vector>

namespace {

struct Nail { float x; float y; };

photo::Rgb stringColor(jint color) {
    return {((uint32_t(color) >> 16) & 255) / 255.f,
            ((uint32_t(color) >> 8) & 255) / 255.f, (uint32_t(color) & 255) / 255.f};
}

template<typename F>
void rasterThread(Nail a, Nail b, int width, int height, float radius, F write) {
    const float dx = b.x - a.x, dy = b.y - a.y;
    const float lengthSquared = dx * dx + dy * dy;
    if (lengthSquared < .0001f) return;
    const bool horizontal = std::abs(dx) >= std::abs(dy);
    const float begin = horizontal ? std::min(a.x, b.x) : std::min(a.y, b.y);
    const float end = horizontal ? std::max(a.x, b.x) : std::max(a.y, b.y);
    const int first = std::max(0, int(std::floor(begin - radius - 1.f)));
    const int last = std::min((horizontal ? width : height) - 1, int(std::ceil(end + radius + 1.f)));
    for (int major = first; major <= last; ++major) {
        if ((major & 1023) == 0 && EncodingInterruption::currentRequested()) return;
        const float t = photo::unit((major - (horizontal ? a.x : a.y)) / (horizontal ? dx : dy));
        const float center = horizontal ? photo::mix(a.y, b.y, t) : photo::mix(a.x, b.x, t);
        const float bound = (radius + 1.f) * 1.414214f;
        const int low = std::max(0, int(std::floor(center - bound)));
        const int high = std::min((horizontal ? height : width) - 1, int(std::ceil(center + bound)));
        for (int minor = low; minor <= high; ++minor) {
            const int x = horizontal ? major : minor, y = horizontal ? minor : major;
            const float position = photo::unit(((x - a.x) * dx + (y - a.y) * dy) / lengthSquared);
            const float distance = std::hypot(x - photo::mix(a.x, b.x, position),
                                               y - photo::mix(a.y, b.y, position));
            const float coverage = photo::smooth(-.65f, .65f, distance + radius) -
                                   photo::smooth(-.65f, .65f, distance - radius);
            if (coverage > 0.f) write(x, y, coverage);
        }
    }
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_nailStringArtImpl(
        JNIEnv *env, jobject, jobject input, jint nails, jint threads, jfloat threadWidth,
        jfloat contrast, jint ink, jint paper, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const int pinCount = std::clamp(int(nails), 32, 256), threadCount = std::clamp(int(threads), 100, 3000);
        const float factor = std::min(1.f, 256.f / std::max(image.width, image.height));
        const int width = std::max(1, int(image.width * factor)), height = std::max(1, int(image.height * factor));
        const float line = std::clamp(threadWidth, .3f, 2.f);
        const float workRadius = .5f * line * std::max(width, height) / 1024.f;
        const float curve = std::clamp(contrast, .25f, 3.f);
        std::vector<float> residual(size_t(width) * height);
        std::vector<Nail> pins(pinCount);
        for (int y = 0; y < height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < width; ++x) {
                residual[size_t(y) * width + x] = std::pow(1.f - photo::luma(image.sample(
                        (x + .5f) * image.width / width - .5f, (y + .5f) * image.height / height - .5f)), curve);
            }
        }
        for (int i = 0; i < pinCount; ++i) {
            const float angle = 2.f * photo::pi * i / pinCount;
            pins[i] = {(width - 1.f) * (.5f + .475f * std::cos(angle)),
                       (height - 1.f) * (.5f + .475f * std::sin(angle))};
        }
        std::vector<int> path{0};
        path.reserve(threadCount + 1);
        int current = 0, previous = -1;
        const float threadDensity = std::max(.025f, 1.6f * workRadius);
        for (int thread = 0; thread < threadCount; ++thread) {
            if (EncodingInterruption::currentRequested()) return;
            int best = -1;
            float bestGain = 0.f;
            for (int candidate = 0; candidate < pinCount; ++candidate) {
                if ((candidate & 15) == 0 && EncodingInterruption::currentRequested()) return;
                const int gap = std::min(std::abs(candidate - current), pinCount - std::abs(candidate - current));
                if (gap < std::max(2, pinCount / 24) || candidate == previous) continue;
                const auto &a = pins[current], &b = pins[candidate];
                const float length = std::hypot(b.x - a.x, b.y - a.y);
                const int samples = std::max(1, int(std::ceil(length)));
                float gain = 0.f;
                for (int i = 0; i < samples; ++i) {
                    const float t = (i + .5f) / samples;
                    const int x = std::clamp(int(photo::mix(a.x, b.x, t) + .5f), 0, width - 1);
                    const int y = std::clamp(int(photo::mix(a.y, b.y, t) + .5f), 0, height - 1);
                    gain += 2.f * residual[size_t(y) * width + x] - threadDensity;
                }
                if (gain > bestGain) { bestGain = gain; best = candidate; }
            }
            if (best < 0) break;
            rasterThread(pins[current], pins[best], width, height, workRadius,
                         [&](int x, int y, float coverage) { residual[size_t(y) * width + x] -= .8f * coverage; });
            previous = current;
            current = best;
            path.push_back(current);
        }
        std::vector<float> density(size_t(image.width) * image.height, 0.f);
        auto outputPin = [&](int index) {
            return Nail{(pins[index].x + .5f) * image.width / width - .5f,
                        (pins[index].y + .5f) * image.height / height - .5f};
        };
        const float radius = .5f * line * image.scale();
        for (size_t i = 1; i < path.size(); ++i) {
            if (EncodingInterruption::currentRequested()) return;
            rasterThread(outputPin(path[i - 1]), outputPin(path[i]), image.width, image.height, radius,
                         [&](int x, int y, float coverage) { density[size_t(y) * image.width + x] += .8f * coverage; });
        }
        const auto inkColor = stringColor(ink), paperColor = stringColor(paper);
        photo::render(image, output, photo::unit(strength), [&](int x, int y, photo::Rgb) {
            return photo::mix(paperColor, inkColor, photo::unit(density[size_t(y) * image.width + x]));
        });
    });
}
