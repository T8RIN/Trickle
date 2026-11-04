#include "PhotoFilter.h"
#include <numeric>
#include <vector>

namespace {

struct MazeEdge {
    int first;
    int second;
    float weight;
    uint8_t direction;
};

photo::Rgb mazeColor(jint color) {
    return {((uint32_t(color) >> 16) & 255) / 255.f,
            ((uint32_t(color) >> 8) & 255) / 255.f,
            (uint32_t(color) & 255) / 255.f};
}

}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_mazePortraitImpl(
        JNIEnv *env, jobject, jobject input, jfloat cellSize, jfloat wallWidth, jfloat bias,
        jint seed, jint ink, jint paper, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const float cell = std::clamp(cellSize, 4.f, 64.f) * image.scale();
        const int columns = std::max(1, std::min(image.width, int(std::ceil(image.width / cell))));
        const int rows = std::max(1, std::min(image.height, int(std::ceil(image.height / cell))));
        const int count = columns * rows;
        const float cellX = float(image.width) / columns, cellY = float(image.height) / rows;
        std::vector<float> tone(count);
        std::vector<int> parent(count), sizes(count, 1);
        std::vector<uint8_t> passages(count, 0);
        std::iota(parent.begin(), parent.end(), 0);
        for (int y = 0; y < rows; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < columns; ++x) {
                tone[y * columns + x] = photo::luma(image.sample((x + .5f) * cellX - .5f,
                                                                 (y + .5f) * cellY - .5f));
            }
        }
        std::vector<MazeEdge> edges;
        edges.reserve(size_t(count) * 2);
        const float edgeBias = std::clamp(bias, 0.f, 8.f);
        for (int y = 0; y < rows; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < columns; ++x) {
                const int a = y * columns + x;
                auto add = [&](int b, uint8_t direction) {
                    const float cost = photo::random(a, b, uint32_t(seed)) +
                                       edgeBias * std::abs(tone[a] - tone[b]);
                    edges.push_back({a, b, cost, direction});
                };
                if (x + 1 < columns) add(a + 1, 2);
                if (y + 1 < rows) add(a + columns, 4);
            }
        }
        // Kruskal preserves exactly one route between every pair of cells.
        std::sort(edges.begin(), edges.end(), [](const MazeEdge &a, const MazeEdge &b) {
            if (a.weight != b.weight) return a.weight < b.weight;
            if (a.first != b.first) return a.first < b.first;
            return a.second < b.second;
        });
        auto root = [&](int index) {
            while (index != parent[index]) {
                parent[index] = parent[parent[index]];
                index = parent[index];
            }
            return index;
        };
        int visited = 0;
        for (const auto &edge : edges) {
            if ((visited++ & 1023) == 0 && EncodingInterruption::currentRequested()) return;
            int a = root(edge.first), b = root(edge.second);
            if (a == b) continue;
            if (sizes[a] < sizes[b]) std::swap(a, b);
            parent[b] = a;
            sizes[a] += sizes[b];
            passages[edge.first] |= edge.direction;
            passages[edge.second] |= edge.direction == 2 ? 8 : 1;
        }
        // Open entrances at opposite corners without adding an interior cycle.
        passages.front() |= 8;
        passages.back() |= 2;
        const float width = std::min(cellX, cellY) * std::clamp(wallWidth, .05f, .48f);
        const auto inkColor = mazeColor(ink), paperColor = mazeColor(paper);
        photo::render(image, output, photo::unit(strength), [&](int x, int y, photo::Rgb) {
            const int cx = std::min(columns - 1, int((x + .5f) / cellX));
            const int cy = std::min(rows - 1, int((y + .5f) / cellY));
            const int index = cy * columns + cx;
            const float px = x + .5f - cx * cellX, py = y + .5f - cy * cellY;
            float coverage = 0.f;
            auto wall = [&](uint8_t direction, float d, int neighbor) {
                if (passages[index] & direction) return;
                const float luminance = neighbor < 0 ? tone[index] : (tone[index] + tone[neighbor]) * .5f;
                const float radius = width * (.06f + .94f * std::pow(1.f - luminance, 2.2f));
                const float inkCoverage = photo::smooth(-.6f, .6f, d + radius) -
                                          photo::smooth(-.6f, .6f, d - radius);
                coverage = std::max(coverage, inkCoverage);
            };
            wall(8, px, cx ? index - 1 : -1);
            wall(2, cellX - px, cx + 1 < columns ? index + 1 : -1);
            wall(1, py, cy ? index - columns : -1);
            wall(4, cellY - py, cy + 1 < rows ? index + columns : -1);
            return photo::mix(paperColor, inkColor, coverage);
        });
    });
}
