#include "PhotoEffects.h"
#include <limits>
#include <numeric>
#include <vector>

namespace photo {
namespace {

float distance(Rgb a, Rgb b) {
    const auto d = a - b;
    return .28f * d.r * d.r + .54f * d.g * d.g + .18f * d.b * d.b;
}

float dotScreen(float x, float y, float angle, float spacing, float ink, float gain) {
    const float u = (x * std::cos(angle) + y * std::sin(angle)) / spacing;
    const float v = (-x * std::sin(angle) + y * std::cos(angle)) / spacing;
    const float dx = fraction(u) - .5f;
    const float dy = fraction(v) - .5f;
    const float radius = std::sqrt(unit(ink + gain * ink * (1.f - ink)) * .5f);
    const float feather = std::min(.24f, .6f / spacing);
    const float dot = 1.f - smooth(radius - feather, radius + feather, std::sqrt(dx * dx + dy * dy));
    return dot * smooth(0.f, .08f, ink);
}

}

void retroHardware(const Image &image, const Output &output, const Params &p, uint32_t, float amount) {
    const bool spectrum = p[0] < .5f;
    const int maxWidth = spectrum ? 256 : 160;
    const int maxHeight = spectrum ? 192 : 144;
    const float factor = std::min(float(maxWidth) / image.width, float(maxHeight) / image.height) / p[1];
    const int width = std::max(1, int(image.width * factor + .5f));
    const int height = std::max(1, int(image.height * factor + .5f));
    std::vector<Rgb> raster(size_t(width) * height);
    constexpr std::array<Rgb, 4> gameBoy = {{{.06f, .22f, .06f}, {.19f, .38f, .19f},
                                            {.55f, .67f, .06f}, {.61f, .74f, .06f}}};
    constexpr int bayer[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
    const auto sample = [&](int x, int y) {
        const auto rgb = image.sample((x + .5f) * image.width / width - .5f,
                                      (y + .5f) * image.height / height - .5f);
        return rgb + gray(((bayer[y & 3][x & 3] + .5f) / 16.f - .5f) * p[2]);
    };
    if (spectrum) {
        std::array<Rgb, 16> palette{};
        for (int i = 0; i < 16; ++i) {
            const float brightness = i < 8 ? .79f : 1.f;
            palette[i] = {i & 2 ? brightness : 0.f, i & 4 ? brightness : 0.f, i & 1 ? brightness : 0.f};
        }
        for (int y = 0; y < height; y += 8) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < width; x += 8) {
                const int blockWidth = std::min(8, width - x);
                const int blockHeight = std::min(8, height - y);
                const int count = blockWidth * blockHeight;
                float errors[64][16];
                for (int i = 0; i < count; ++i) {
                    const auto rgb = sample(x + i % blockWidth, y + i / blockWidth);
                    for (int color = 0; color < 16; ++color) errors[i][color] = distance(rgb, palette[color]);
                }
                float best = std::numeric_limits<float>::infinity();
                int ink = 0, paper = 0;
                for (int a = 0; a < 16; ++a) {
                    // The two colors in an attribute cell share their BRIGHT bit.
                    for (int b = a; b < (a / 8 + 1) * 8; ++b) {
                        float error = 0.f;
                        for (int i = 0; i < count; ++i) error += std::min(errors[i][a], errors[i][b]);
                        if (error < best) {
                            best = error;
                            paper = a;
                            ink = b;
                        }
                    }
                }
                for (int i = 0; i < count; ++i) {
                    raster[size_t(y + i / blockWidth) * width + x + i % blockWidth] =
                            palette[errors[i][ink] < errors[i][paper] ? ink : paper];
                }
            }
        }
    } else {
        for (int y = 0; y < height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < width; ++x) {
                const float brightness = unit(luma(sample(x, y)));
                raster[size_t(y) * width + x] = gameBoy[int(brightness * 3.f + .5f)];
            }
        }
    }
    render(image, output, amount, [&](int x, int y, Rgb) {
        return raster[size_t(int64_t(y) * height / image.height) * width + int64_t(x) * width / image.width];
    });
}

void risograph(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const float scale = image.scale();
    const float spacing = std::max(2.f, p[0] * scale);
    const float shift = p[1] * scale;
    constexpr std::array<Rgb, 3> inks = {{{.07f, .34f, .68f}, {.94f, .18f, .34f}, {.35f, .66f, .18f}}};
    const int scheme = int(p[3]);
    const auto cool = inks[scheme == 2 ? 2 : 0];
    const auto warm = inks[scheme == 1 ? 2 : 1];
    render(image, output, amount, [&](int x, int y, Rgb original) {
        const auto offset = image.sample(x + shift, y - shift * .55f);
        const float a = unit((1.f - luma(original)) * .85f + .5f * (original.b - original.r));
        const float b = unit((1.f - luma(offset)) * .7f + .65f * (offset.r - offset.b));
        const float first = dotScreen(float(x), float(y), pi / 12.f, spacing, a, p[2]);
        const float second = dotScreen(x + shift, y - shift * .55f, pi / 3.f, spacing, b, p[2]);
        const auto c = mix(gray(1.f), cool, first);
        const auto m = mix(gray(1.f), warm, second);
        const float paper = .96f - p[4] * .14f * random(x, y, seed);
        return Rgb{c.r * m.r, c.g * m.g * .98f, c.b * m.b * .93f} * paper;
    });
}

void cmykPress(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const float scale = image.scale();
    const float spacing = std::max(2.f, p[0] * scale);
    const float shift = p[3] * scale;
    constexpr float angle[] = {15.f, 75.f, 0.f, 45.f};
    constexpr float xShift[] = {-1.f, 1.f, .25f, 0.f};
    constexpr float yShift[] = {.6f, -.6f, 1.f, 0.f};
    render(image, output, amount, [&](int x, int y, Rgb) {
        std::array<float, 4> dot{};
        for (int plate = 0; plate < 4; ++plate) {
            const float sx = x + shift * xShift[plate];
            const float sy = y + shift * yShift[plate];
            const auto rgb = image.sample(sx, sy);
            const float black = (1.f - std::max({rgb.r, rgb.g, rgb.b})) * p[1];
            const float chroma[] = {1.f - rgb.r - black, 1.f - rgb.g - black, 1.f - rgb.b - black};
            const float total = black + chroma[0] + chroma[1] + chroma[2];
            const float coverage = plate == 3 ? black : chroma[plate];
            dot[plate] = dotScreen(sx, sy, angle[plate] * pi / 180.f, spacing,
                                   coverage * std::min(1.f, p[4] / std::max(total, .001f)), p[2]);
        }
        const float paper = .98f - .065f * p[5] * random(x, y, seed);
        const float black = 1.f - .92f * dot[3];
        return Rgb{1.f - .94f * dot[0], 1.f - .94f * dot[1], 1.f - .94f * dot[2]} * (black * paper);
    });
}

void thermalReceipt(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const float cell = std::max(1.f, p[0] * image.scale());
    const int columns = int(std::ceil(image.width / cell));
    std::vector<float> heat(columns), density(columns);
    int lastRow = -1;
    render(image, output, amount, [&](int x, int y, Rgb) {
        const int row = int(y / cell);
        if (lastRow != row) {
            lastRow = row;
            for (int i = 0; i < columns; ++i) {
                if ((i & 2047) == 0 && EncodingInterruption::currentRequested()) break;
                const float power = (1.f - luma(image.sample((i + .5f) * cell - .5f, (row + .5f) * cell - .5f))) * p[1];
                const float broken = random(i, 0, seed) < p[3] ? .26f : 1.f;
                heat[i] = mix(power * broken, heat[i], p[2]);
                const float paper = .1f * (random(i, row, seed + 1) - .5f);
                density[i] = smooth(.16f + paper, .76f + paper, heat[i]) * (1.f - p[4]);
            }
        }
        const float dot = dotScreen(x + .5f, y + .5f, 0.f, cell, density[int(x / cell)], .12f);
        return mix(Rgb{.98f, .965f, .915f}, Rgb{.14f, .12f, .105f}, dot);
    });
}

void crossStitch(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const float cell = std::max(4.f, p[0] * image.scale());
    const float edge = std::min(.2f, .65f / cell);
    const float angle = p[3] * pi / 180.f;
    const float light = std::cos(angle) * .65f + std::sin(angle) * .35f;
    const int columns = int(std::ceil(image.width / cell));
    const int rows = int(std::ceil(image.height / cell));
    constexpr int bins = 12;
    constexpr int paletteSize = bins * bins * bins;
    std::array<int, paletteSize> counts{};
    std::array<Rgb, paletteSize> sums{};
    std::vector<uint16_t> cells(size_t(columns) * rows);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < columns; ++x) {
            if ((x & 2047) == 0 && EncodingInterruption::currentRequested()) return;
            const auto color = image.sample((x + .5f) * cell - .5f, (y + .5f) * cell - .5f);
            auto bin = [](float value) { return std::min(bins - 1, int(value * bins)); };
            const int index = (bin(color.r) * bins + bin(color.g)) * bins + bin(color.b);
            cells[size_t(y) * columns + x] = index;
            ++counts[index];
            sums[index] = sums[index] + color;
        }
    }
    std::array<int, paletteSize> order{};
    std::iota(order.begin(), order.end(), 0);
    const int colors = int(p[1]);
    std::partial_sort(order.begin(), order.begin() + colors, order.end(), [&](int a, int b) {
        return counts[a] > counts[b] || (counts[a] == counts[b] && a < b);
    });
    std::array<Rgb, paletteSize> threads{};
    for (int i = 0; i < paletteSize; ++i) {
        if (!counts[i]) continue;
        const auto target = sums[i] * (1.f / counts[i]);
        float best = std::numeric_limits<float>::infinity();
        for (int j = 0; j < colors && counts[order[j]]; ++j) {
            const auto color = sums[order[j]] * (1.f / counts[order[j]]);
            const float error = distance(target, color);
            if (error < best) {
                best = error;
                threads[i] = color;
            }
        }
    }
    render(image, output, amount, [&](int x, int y, Rgb) {
        const float u = (x + .5f) / cell;
        const float v = (y + .5f) / cell;
        const float dx = fraction(u) - .5f;
        const float dy = fraction(v) - .5f;
        const auto thread = threads[cells[size_t(int(v)) * columns + int(u)]];
        const float grain = random(x, y, seed) - .5f;
        const float weave = .045f * std::cos(2.f * pi * u * 4.f) * std::cos(2.f * pi * v * 4.f);
        auto result = Rgb{.87f, .81f, .71f} * (1.f + weave + grain * .045f);
        auto stitch = [&](float along, float across) {
            const float distance = std::hypot(across, std::max(std::abs(along) - .49f, 0.f));
            const float width = p[2];
            const float mask = 1.f - smooth(width - edge, width + edge, distance);
            const float relief = unit(1.f - std::abs(across / width));
            const float shine = 1.f + p[4] * (.26f * relief + light * across / width - .28f);
            const float fibers = .07f * std::cos(2.f * pi * (across * 14.f + along * 2.f)) + .05f * grain;
            result = mix(result, thread * (shine + fibers), mask);
        };
        stitch((dx + dy) * .7071f, (dx - dy) * .7071f);
        stitch((dx - dy) * .7071f, (dx + dy) * .7071f);
        return result;
    });
}

void autostereogram(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const int width = image.width;
    if (width < 8) {
        render(image, output, 0.f, [](int, int, Rgb original) { return original; });
        return;
    }
    const int repeat = std::clamp(int(width * p[0]), 4, width - 1);
    const float scale = image.scale();
    const float dots = std::max(1.f, p[3] * scale);
    const float soften = p[4] * scale;
    std::vector<int> links(width), leftMatch(width), rightMatch(width);
    std::vector<int> next(width);
    std::array<int, 32> bucket{};
    auto root = [&](int x) {
        while (links[x] != x) {
            links[x] = links[links[x]];
            x = links[x];
        }
        return x;
    };
    for (int y = 0; y < image.height; ++y) {
        bucket.fill(-1);
        for (int x = 0; x < width; ++x) {
            if ((x & 2047) == 0 && EncodingInterruption::currentRequested()) return;
            links[x] = x;
            leftMatch[x] = rightMatch[x] = -1;
            const float depth = luma(image.read(x, y)) * .5f +
                                .125f * (luma(image.sample(x - soften, float(y))) +
                                         luma(image.sample(x + soften, float(y))) +
                                         luma(image.sample(float(x), y - soften)) +
                                         luma(image.sample(float(x), y + soften)));
            const int level = int(std::pow(unit(p[5] >= .5f ? 1.f - depth : depth), p[2]) * 31.f + .5f);
            next[x] = bucket[level];
            bucket[level] = x;
        }
        for (int level = 31; level >= 0; --level) {
            const int separation = std::max(2, int(repeat * (1.f - p[1] * level / 31.f)));
            for (int center = bucket[level]; center >= 0; center = next[center]) {
                if ((center & 2047) == 0 && EncodingInterruption::currentRequested()) return;
                const int left = center - separation / 2;
                const int right = left + separation;
                if (left < 0 || right >= width || rightMatch[left] >= 0 || leftMatch[right] >= 0) continue;
                rightMatch[left] = right;
                leftMatch[right] = left;
                links[root(right)] = root(left);
            }
        }
        for (int x = 0; x < width; ++x) {
            if ((x & 2047) == 0 && EncodingInterruption::currentRequested()) return;
            const int origin = root(x);
            const auto bits = hash(int(origin / dots), int(y / dots), seed);
            const Rgb color{.18f + .75f * (bits & 255u) / 255.f,
                            .18f + .75f * ((bits >> 8u) & 255u) / 255.f,
                            .18f + .75f * ((bits >> 16u) & 255u) / 255.f};
            output.write(x, y, mix(image.read(x, y), color, amount), image.pixel(x, y)[3]);
        }
    }
}

}
