#include "PhotoFilter.h"
#include <vector>

namespace {

struct RibbonPoint {
    float x;
    float y;
    float radius;
};

photo::Rgb ribbonColor(jint color) {
    return {((uint32_t(color) >> 16) & 255) / 255.f,
            ((uint32_t(color) >> 8) & 255) / 255.f,
            (uint32_t(color) & 255) / 255.f};
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_hilbertRibbonImpl(
        JNIEnv *env, jobject, jobject input, jint order, jfloat lineWidth, jfloat contrast,
        jint ink, jint paper, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const int side = 1 << std::clamp(int(order), 2, 9);
        const float cellX = float(image.width) / side;
        const float cellY = float(image.height) / side;
        const float radius = .56f * std::min(cellX, cellY) * std::clamp(lineWidth, .1f, 1.f);
        const float curve = std::clamp(contrast, .25f, 3.f);
        std::vector<RibbonPoint> points(size_t(side) * side);
        std::vector<int> indices(points.size());
        for (int i = 0; i < int(points.size()); ++i) {
            if ((i & 1023) == 0 && EncodingInterruption::currentRequested()) return;
            int x = 0, y = 0, index = i;
            for (int size = 1; size < side; size *= 2) {
                const int rx = (index >> 1) & 1;
                const int ry = (index ^ rx) & 1;
                if (!ry) {
                    if (rx) { x = size - 1 - x; y = size - 1 - y; }
                    std::swap(x, y);
                }
                x += size * rx;
                y += size * ry;
                index >>= 2;
            }
            const float px = (x + .5f) * cellX - .5f;
            const float py = (y + .5f) * cellY - .5f;
            const float tone = std::pow(1.f - photo::luma(image.sample(px, py)), curve);
            points[i] = {px, py, radius * (.025f + .975f * tone)};
            indices[size_t(y) * side + x] = i;
        }
        const auto inkColor = ribbonColor(ink), paperColor = ribbonColor(paper);
        photo::render(image, output, photo::unit(strength), [&](int x, int y, photo::Rgb) {
            const int cx = std::min(side - 1, int((x + .5f) / cellX));
            const int cy = std::min(side - 1, int((y + .5f) / cellY));
            float coverage = 0.f;
            for (int row = std::max(0, cy - 1); row <= std::min(side - 1, cy + 1); ++row) {
                for (int col = std::max(0, cx - 1); col <= std::min(side - 1, cx + 1); ++col) {
                    const int index = indices[size_t(row) * side + col];
                    if (index + 1 == int(points.size())) continue;
                    const auto &a = points[index], &b = points[index + 1];
                    const float dx = b.x - a.x, dy = b.y - a.y;
                    const float t = photo::unit(((x - a.x) * dx + (y - a.y) * dy) /
                                               (dx * dx + dy * dy));
                    const float px = x - photo::mix(a.x, b.x, t);
                    const float py = y - photo::mix(a.y, b.y, t);
                    const float distance = std::hypot(px, py);
                    const float thickness = photo::mix(a.radius, b.radius, t);
                    const float inkCoverage = photo::smooth(-.65f, .65f, distance + thickness) -
                                              photo::smooth(-.65f, .65f, distance - thickness);
                    coverage = std::max(coverage, inkCoverage);
                }
            }
            return photo::mix(paperColor, inkColor, coverage);
        });
    });
}
