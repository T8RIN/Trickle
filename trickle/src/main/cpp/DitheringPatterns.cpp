#include <jni.h>
#include <android/api-level.h>
#include <android/bitmap.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <new>
#include "BitmapUtils.h"
#include "lowpoly/cairo/dither/blue-noise-64x64.h"

namespace {

// Matches DitheringType starting at BayerSixteen.
enum class DitherPattern {
    BayerSixteen,
    BlueNoise,
    InterleavedGradientNoise,
    ClusteredDot,
    Diamond,
    HorizontalLines,
    VerticalLines,
    DiagonalLines,
    CrosshatchMatrix,
    SerpentineFloydSteinberg,
    SerpentineAtkinson,
    SerpentineBurkes,
    Riemersma,
    Ostromoukhov,
    ShiauFan,
    SpiralHalftone,
    HexagonalDot,
    WaveLines
};

constexpr uint8_t clusteredDot[8][8] = {
        {60, 56, 44, 38, 32, 48, 52, 62},
        {58, 35, 30, 18, 22, 26, 41, 54},
        {45, 28, 12,  9,  5, 14, 24, 49},
        {42, 19, 11,  1,  3,  7, 23, 36},
        {33, 20,  4,  2,  0,  8, 16, 39},
        {50, 27, 15,  6, 10, 13, 31, 46},
        {53, 40, 25, 21, 17, 29, 34, 57},
        {63, 55, 51, 37, 43, 47, 59, 61}
};

constexpr uint8_t diamond[8][8] = {
        {60, 56, 41, 32, 24, 46, 52, 62},
        {58, 43, 36, 16, 21, 28, 51, 54},
        {42, 34, 12,  9,  5, 20, 26, 47},
        {38, 17, 11,  1,  3,  7, 22, 30},
        {25, 18,  4,  2,  0,  8, 13, 33},
        {49, 29, 23,  6, 10, 15, 37, 44},
        {53, 48, 27, 19, 14, 35, 40, 57},
        {63, 55, 50, 31, 39, 45, 59, 61}
};

constexpr uint8_t crosshatchMatrix[8][8] = {
        { 0, 14, 38, 56, 62, 54, 40, 16},
        {18,  2, 34, 50, 58, 46, 36,  6},
        {42, 26, 10, 22, 44, 28, 12, 24},
        {60, 52, 30,  4, 20,  8, 32, 48},
        {63, 55, 41, 17,  1, 15, 39, 57},
        {59, 47, 37,  7, 19,  3, 35, 51},
        {45, 29, 13, 25, 43, 27, 11, 23},
        {21,  9, 33, 49, 61, 53, 31,  5}
};

// Ranks ordered by spiral phase around the center of the tile.
constexpr uint8_t spiralHalftone[12][12] = {
        { 77,  46,  22, 142, 126, 116, 111, 114, 122, 135,  15,  41},
        { 51,  20, 132, 109,  88,  76,  67,  74,  84, 101, 128,  10},
        { 27, 137, 107,  79,  53,  36,  25,  33,  47,  71,  96, 130},
        { 14, 120,  83,  49,  21, 138, 131, 136,  13,  43,  73, 106},
        {  6, 110,  69,  28, 134, 104,  91,  98, 129,  17,  52,  90},
        {  3, 103,  61,  23, 127,  81,  45,  68, 102,   0,  40,  80},
        {  8, 112,  72,  30, 140, 117,   9,  55,  95, 133,  31,  75},
        { 18, 124,  89,  57,  26,  19,  32,  62, 100, 141,  38,  78},
        { 34,   1, 115,  85,  64,  59,  66,  93, 121,  11,  48,  86},
        { 58,  24, 143, 119, 105,  97, 108, 125,   7,  35,  65,  99},
        { 82,  56,  29,  12,   2, 139,   4,  16,  37,  60,  92, 123},
        {113,  87,  63,  50,  42,  39,  44,  54,  70,  94, 118,   5}
};

// Ranks ordered by distance to the nearest dot on a staggered lattice.
constexpr uint8_t hexagonalDot[14][8] = {
        {68,  56,  52, 72, 90, 108, 102, 94},
        {38,  24,  28, 33, 66, 100, 110, 62},
        {16,   8,   4, 20, 46,  86,  82, 50},
        {14,   0,   2, 12, 41,  77,  79, 37},
        {17,   9,   5, 21, 47,  87,  83, 51},
        {39,  25,  29, 34, 67, 101, 111, 63},
        {69,  57,  53, 73, 91, 109, 103, 95},
        {92,  96, 106, 88, 74,  54,  58, 70},
        {60, 104,  98, 64, 35,  30,  26, 42},
        {48,  80,  84, 44, 22,   6,  10, 18},
        {32,  78,  76, 40, 13,   3,   1, 15},
        {49,  81,  85, 45, 23,   7,  11, 19},
        {61, 105,  99, 65, 36,  31,  27, 43},
        {93,  97, 107, 89, 75,  55,  59, 71}
};

struct DiffusionTap {
    int x;
    int y;
    int weight;
};

struct DiffusionKernel {
    int rows;
    int divisor;
    int count;
    DiffusionTap taps[7];
};

constexpr DiffusionKernel floydSteinberg = {
        2, 16, 4, {{1, 0, 7}, {-1, 1, 3}, {0, 1, 5}, {1, 1, 1}}
};
constexpr DiffusionKernel atkinson = {
        3, 8, 6, {{1, 0, 1}, {2, 0, 1}, {-1, 1, 1}, {0, 1, 1}, {1, 1, 1}, {0, 2, 1}}
};
constexpr DiffusionKernel burkes = {
        2, 32, 7, {{1, 0, 8}, {2, 0, 4}, {-2, 1, 2}, {-1, 1, 4}, {0, 1, 8}, {1, 1, 4}, {2, 1, 2}}
};
constexpr DiffusionKernel shiauFan = {
        2, 16, 5, {{1, 0, 8}, {-3, 1, 1}, {-2, 1, 1}, {-1, 1, 2}, {0, 1, 4}}
};
constexpr DiffusionKernel ostromoukhov = {
        2, 1, 3, {{1, 0, 0}, {-1, 1, 0}, {0, 1, 0}}
};

struct OstromoukhovWeights {
    float weights[3];

    constexpr OstromoukhovWeights(int right, int downLeft, int down)
            : weights{static_cast<float>(right) / (right + downLeft + down),
                      static_cast<float>(downLeft) / (right + downLeft + down),
                      static_cast<float>(down) / (right + downLeft + down)} {}
};

// Appendix I of Ostromoukhov's A Simple and Efficient Error-Diffusion Algorithm.
// Levels 128 through 255 mirror levels 127 through 0.
constexpr OstromoukhovWeights ostromoukhovWeights[] = {
        {13, 0, 5},         {13, 0, 5},         {21, 0, 10},        {7, 0, 4},
        {8, 0, 5},          {47, 3, 28},        {23, 3, 13},        {15, 3, 8},
        {22, 6, 11},        {43, 15, 20},       {7, 3, 3},          {501, 224, 211},
        {249, 116, 103},    {165, 80, 67},      {123, 62, 49},      {489, 256, 191},
        {81, 44, 31},       {483, 272, 181},    {60, 35, 22},       {53, 32, 19},
        {237, 148, 83},     {471, 304, 161},    {3, 2, 1},          {481, 314, 185},
        {354, 226, 155},    {1389, 866, 685},   {227, 138, 125},    {267, 158, 163},
        {327, 188, 220},    {61, 34, 45},       {627, 338, 505},    {1227, 638, 1075},
        {20, 10, 19},       {1937, 1000, 1767}, {977, 520, 855},    {657, 360, 551},
        {71, 40, 57},       {2005, 1160, 1539}, {337, 200, 247},    {2039, 1240, 1425},
        {257, 160, 171},    {691, 440, 437},    {1045, 680, 627},   {301, 200, 171},
        {177, 120, 95},     {2141, 1480, 1083}, {1079, 760, 513},   {725, 520, 323},
        {137, 100, 57},     {2209, 1640, 855},  {53, 40, 19},       {2243, 1720, 741},
        {565, 440, 171},    {759, 600, 209},    {1147, 920, 285},   {2311, 1880, 513},
        {97, 80, 19},       {335, 280, 57},     {1181, 1000, 171},  {793, 680, 95},
        {599, 520, 57},     {2413, 2120, 171},  {405, 360, 19},     {2447, 2200, 57},
        {11, 10, 0},        {158, 151, 3},      {178, 179, 7},      {1030, 1091, 63},
        {248, 277, 21},     {318, 375, 35},     {458, 571, 63},     {878, 1159, 147},
        {5, 7, 1},          {172, 181, 37},     {97, 76, 22},       {72, 41, 17},
        {119, 47, 29},      {4, 1, 1},          {4, 1, 1},          {4, 1, 1},
        {4, 1, 1},          {4, 1, 1},          {4, 1, 1},          {4, 1, 1},
        {4, 1, 1},          {4, 1, 1},          {65, 18, 17},       {95, 29, 26},
        {185, 62, 53},      {30, 11, 9},        {35, 14, 11},       {85, 37, 28},
        {55, 26, 19},       {80, 41, 29},       {155, 86, 59},      {5, 3, 2},
        {5, 3, 2},          {5, 3, 2},          {5, 3, 2},          {5, 3, 2},
        {5, 3, 2},          {5, 3, 2},          {5, 3, 2},          {5, 3, 2},
        {5, 3, 2},          {5, 3, 2},          {5, 3, 2},          {5, 3, 2},
        {305, 176, 119},    {155, 86, 59},      {105, 56, 39},      {80, 41, 29},
        {65, 32, 23},       {55, 26, 19},       {335, 152, 113},    {85, 37, 28},
        {115, 48, 37},      {35, 14, 11},       {355, 136, 109},    {30, 11, 9},
        {365, 128, 107},    {185, 62, 53},      {25, 8, 7},         {95, 29, 26},
        {385, 112, 103},    {65, 18, 17},       {395, 104, 101},    {4, 1, 1}
};
static_assert(sizeof(ostromoukhovWeights) / sizeof(ostromoukhovWeights[0]) == 128,
              "Ostromoukhov requires one weight set per level from 0 to 127");

const DiffusionKernel *diffusionKernel(DitherPattern pattern) {
    switch (pattern) {
        case DitherPattern::SerpentineFloydSteinberg: return &floydSteinberg;
        case DitherPattern::SerpentineAtkinson: return &atkinson;
        case DitherPattern::SerpentineBurkes: return &burkes;
        case DitherPattern::Ostromoukhov: return &ostromoukhov;
        case DitherPattern::ShiauFan: return &shiauFan;
        default: return nullptr;
    }
}

int bayerRank(int x, int y) {
    int rank = 0;
    for (int bit = 0; bit < 4; ++bit) {
        rank = 4 * rank + 2 * ((x ^ y) & 1) + (y & 1);
        x >>= 1;
        y >>= 1;
    }
    return rank;
}

int lineRank(int across, int along) {
    constexpr int acrossRanks[] = {0, 2, 4, 6, 7, 5, 3, 1};
    constexpr int alongRanks[] = {0, 4, 2, 6, 1, 5, 3, 7};
    return 8 * acrossRanks[across % 8] + alongRanks[along % 8];
}

float rankOffset(int rank, int levels) {
    return (rank + 0.5f) * (255.f / levels) - 127.5f;
}

float thresholdOffset(DitherPattern pattern, int x, int y) {
    switch (pattern) {
        case DitherPattern::BayerSixteen:
            return rankOffset(bayerRank(x, y), 256);
        case DitherPattern::BlueNoise: {
            int rank = dither_blue_noise_64x64[(y % 64) * 64 + x % 64];
            return rankOffset(rank, 4096);
        }
        case DitherPattern::InterleavedGradientNoise: {
            // Jorge Jimenez, Next Generation Post Processing in Call of Duty: Advanced Warfare.
            double gradient = 0.06711056 * x + 0.00583715 * y;
            double noise = 52.9829189 * (gradient - std::floor(gradient));
            return static_cast<float>(255.0 * (noise - std::floor(noise) - 0.5));
        }
        case DitherPattern::ClusteredDot:
            return rankOffset(clusteredDot[y % 8][x % 8], 64);
        case DitherPattern::Diamond:
            return rankOffset(diamond[y % 8][x % 8], 64);
        case DitherPattern::HorizontalLines:
            return rankOffset(lineRank(y, x), 64);
        case DitherPattern::VerticalLines:
            return rankOffset(lineRank(x, y), 64);
        case DitherPattern::DiagonalLines:
            return rankOffset(lineRank(x % 8 + y % 8, x), 64);
        case DitherPattern::CrosshatchMatrix:
            return rankOffset(crosshatchMatrix[y % 8][x % 8], 64);
        case DitherPattern::SpiralHalftone:
            return rankOffset(spiralHalftone[y % 12][x % 12], 144);
        case DitherPattern::HexagonalDot:
            return rankOffset(hexagonalDot[y % 14][x % 8], 112);
        case DitherPattern::WaveLines: {
            constexpr int wave[] = {3, 4, 5, 5, 6, 6, 6, 6, 6, 5, 5, 4,
                                    3, 2, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2};
            return rankOffset(lineRank(y % 8 + wave[x % 24], x), 64);
        }
        default:
            return 0.f;
    }
}

using Channels = std::array<float, 3>;

struct DitherBitmap {
    const uint8_t *source;
    uint8_t *output;
    uint32_t sourceStride;
    uint32_t outputStride;
    int width;
    int height;
    int alphaMode;
    int channelCount;
};

int bitmapAlphaMode(JNIEnv *env, jobject bitmap, const AndroidBitmapInfo &info) {
    static const int apiLevel = android_get_device_api_level();
    if (apiLevel >= 30) {
        return info.flags & ANDROID_BITMAP_FLAGS_ALPHA_MASK;
    }

    // The NDK bitmap flags are available from API 30.
    jclass bitmapClass = env->GetObjectClass(bitmap);
    if (!bitmapClass) {
        return ANDROID_BITMAP_FLAGS_ALPHA_PREMUL;
    }
    jmethodID hasAlpha = env->GetMethodID(bitmapClass, "hasAlpha", "()Z");
    if (!hasAlpha || !env->CallBooleanMethod(bitmap, hasAlpha)) {
        return ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE;
    }
    jmethodID isPremultiplied = env->GetMethodID(bitmapClass, "isPremultiplied", "()Z");
    return isPremultiplied && env->CallBooleanMethod(bitmap, isPremultiplied)
           ? ANDROID_BITMAP_FLAGS_ALPHA_PREMUL
           : ANDROID_BITMAP_FLAGS_ALPHA_UNPREMUL;
}

uint8_t pixelAlpha(const uint8_t *pixel, int alphaMode) {
    return alphaMode == ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE ? 255 : pixel[3];
}

Channels readColor(const uint8_t *src, uint8_t alpha, int alphaMode, int channelCount) {
    Channels color = {static_cast<float>(src[0]), static_cast<float>(src[1]),
                      static_cast<float>(src[2])};
    if (alphaMode == ANDROID_BITMAP_FLAGS_ALPHA_PREMUL && alpha != 255) {
        for (auto &channel : color) {
            channel = channel * 255.f / alpha;
        }
    }
    if (channelCount == 1) {
        color[0] = (299 * color[0] + 587 * color[1] + 114 * color[2]) / 1000.f;
    }
    return color;
}

Channels quantize(
        uint8_t *dst, uint8_t alpha, const Channels &color, const Channels &correction,
        int channelCount, float cutoff
) {
    Channels levels{};
    dst[3] = alpha;
    for (int channel = 0; channel < channelCount; ++channel) {
        bool high = color[channel] + correction[channel] >= cutoff;
        dst[channel] = high ? alpha : 0;
        levels[channel] = high ? 255.f : 0.f;
    }
    if (channelCount == 1) {
        dst[1] = dst[2] = dst[0];
    }
    return levels;
}

float clampCutoff(float cutoff) {
    return std::max(1.f, std::min(255.f, cutoff));
}

void applyDithering(
        const DitherBitmap &bitmap, DitherPattern pattern, int threshold,
        const DiffusionKernel *kernel, Channels *errors
) {
    int width = bitmap.width;
    int height = bitmap.height;
    int channelCount = bitmap.channelCount;
    bool adaptive = pattern == DitherPattern::Ostromoukhov;
    const Channels zero{};
    for (int y = 0; y < height; ++y) {
        const auto *sourceRow = bitmap.source + static_cast<size_t>(y) * bitmap.sourceStride;
        auto *outputRow = bitmap.output + static_cast<size_t>(y) * bitmap.outputStride;
        int step = kernel && y % 2 ? -1 : 1;
        Channels *errorRows[3] = {};
        if (kernel) {
            // Reuse the current row once its errors have been diffused to future pixels.
            for (int row = 0; row < kernel->rows && row < height - y; ++row) {
                errorRows[row] = errors + static_cast<size_t>((y + row) % kernel->rows) * width;
            }
        }

        for (int x = step > 0 ? 0 : width - 1; x >= 0 && x < width; x += step) {
            const auto *src = sourceRow + static_cast<size_t>(x) * 4;
            auto *dst = outputRow + static_cast<size_t>(x) * 4;
            uint8_t alpha = pixelAlpha(src, bitmap.alphaMode);
            if (alpha == 0) {
                std::fill_n(dst, 4, 0);
                continue;
            }

            auto color = readColor(src, alpha, bitmap.alphaMode, channelCount);
            const auto &correction = kernel ? errorRows[0][x] : zero;
            float cutoff = clampCutoff(threshold + thresholdOffset(pattern, x, y));
            auto levels = quantize(dst, alpha, color, correction, channelCount, cutoff);

            if (kernel) {
                Channels error;
                const OstromoukhovWeights *weights[3];
                for (int channel = 0; channel < channelCount; ++channel) {
                    error[channel] = color[channel] + correction[channel] - levels[channel];
                    if (adaptive) {
                        int level = std::min(255, static_cast<int>(color[channel] + 0.5f));
                        weights[channel] = &ostromoukhovWeights[std::min(level, 255 - level)];
                    }
                }
                for (int i = 0; i < kernel->count; ++i) {
                    const auto &tap = kernel->taps[i];
                    int targetX = x + step * tap.x;
                    if (targetX < 0 || targetX >= width || !errorRows[tap.y]) {
                        continue;
                    }
                    float fixedWeight = static_cast<float>(tap.weight) / kernel->divisor;
                    auto &target = errorRows[tap.y][targetX];
                    for (int channel = 0; channel < channelCount; ++channel) {
                        float weight = adaptive ? weights[channel]->weights[i] : fixedWeight;
                        target[channel] += error[channel] * weight;
                    }
                }
            }
        }
        if (kernel) {
            std::fill_n(errorRows[0], width, Channels{});
        }
    }
}

template<typename Visitor>
void visitHilbert(
        int64_t x, int64_t y, int64_t ax, int64_t ay, int64_t bx, int64_t by, int64_t size,
        int width, int height, const Visitor &visit
) {
    // The two axis vectors describe a rotated or reflected square in the padded image.
    int64_t left = std::min(x, x + ax + bx);
    int64_t top = std::min(y, y + ay + by);
    if (left >= width || top >= height) {
        return;
    }
    if (size == 1) {
        visit(static_cast<int>(left), static_cast<int>(top));
        return;
    }

    size /= 2;
    ax /= 2;
    ay /= 2;
    bx /= 2;
    by /= 2;
    visitHilbert(x, y, bx, by, ax, ay, size, width, height, visit);
    visitHilbert(x + ax, y + ay, ax, ay, bx, by, size, width, height, visit);
    visitHilbert(x + ax + bx, y + ay + by, ax, ay, bx, by, size, width, height, visit);
    visitHilbert(x + ax + 2 * bx, y + ay + 2 * by, -bx, -by, -ax, -ay, size, width, height, visit);
}

void applyRiemersma(const DitherBitmap &bitmap, int threshold) {
    constexpr int historySize = 16;
    constexpr float oldestWeight = 1.f / 16;
    constexpr float decay = 0.8312379f; // Sixteen entries from weight 1 down to 1/16.
    Channels history[historySize]{};
    Channels correction{};
    int oldest = 0;
    float cutoff = clampCutoff(threshold);
    int64_t size = 1;
    while (size < std::max(bitmap.width, bitmap.height)) {
        size *= 2;
    }

    visitHilbert(0, 0, size, 0, 0, size, size, bitmap.width, bitmap.height, [&](int x, int y) {
        const auto *src = bitmap.source + static_cast<size_t>(y) * bitmap.sourceStride +
                          static_cast<size_t>(x) * 4;
        auto *dst = bitmap.output + static_cast<size_t>(y) * bitmap.outputStride +
                    static_cast<size_t>(x) * 4;
        uint8_t alpha = pixelAlpha(src, bitmap.alphaMode);
        Channels error{};
        if (alpha == 0) {
            std::fill_n(dst, 4, 0);
        } else {
            auto color = readColor(src, alpha, bitmap.alphaMode, bitmap.channelCount);
            auto levels = quantize(dst, alpha, color, correction, bitmap.channelCount, cutoff);
            for (int channel = 0; channel < bitmap.channelCount; ++channel) {
                error[channel] = color[channel] - levels[channel];
            }
        }
        for (int channel = 0; channel < bitmap.channelCount; ++channel) {
            correction[channel] = decay * (correction[channel] - oldestWeight * history[oldest][channel])
                                  + error[channel];
        }
        history[oldest] = error;
        oldest = (oldest + 1) % historySize;
    });
}

} // namespace

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_DitheringPipelineImpl_ditheringPatternImpl(
        JNIEnv *env, jobject, jobject input, jint patternId, jint threshold, jboolean isGrayScale
) {
    AndroidBitmapInfo sourceInfo;
    if (AndroidBitmap_getInfo(env, input, &sourceInfo) < 0 ||
        sourceInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return nullptr;
    }
    int alphaMode = bitmapAlphaMode(env, input, sourceInfo);
    if (env->ExceptionCheck()) {
        return nullptr;
    }

    auto pattern = static_cast<DitherPattern>(patternId);
    const auto *kernel = diffusionKernel(pattern);
    std::unique_ptr<Channels[]> errors;
    if (kernel) {
        errors.reset(new (std::nothrow) Channels[static_cast<size_t>(sourceInfo.width) * kernel->rows]());
        if (!errors) {
            return nullptr;
        }
    }

    jobject output = createBitmap(env, sourceInfo.width, sourceInfo.height);
    AndroidBitmapInfo outputInfo;
    if (!output || AndroidBitmap_getInfo(env, output, &outputInfo) < 0) {
        return nullptr;
    }

    void *sourcePixels;
    if (AndroidBitmap_lockPixels(env, input, &sourcePixels) < 0) {
        return nullptr;
    }
    void *outputPixels;
    if (AndroidBitmap_lockPixels(env, output, &outputPixels) < 0) {
        AndroidBitmap_unlockPixels(env, input);
        return nullptr;
    }

    DitherBitmap bitmap{
            static_cast<const uint8_t *>(sourcePixels), static_cast<uint8_t *>(outputPixels),
            sourceInfo.stride, outputInfo.stride,
            static_cast<int>(sourceInfo.width), static_cast<int>(sourceInfo.height),
            alphaMode, isGrayScale ? 1 : 3
    };
    if (pattern == DitherPattern::Riemersma) {
        applyRiemersma(bitmap, threshold);
    } else {
        applyDithering(bitmap, pattern, threshold, kernel, errors.get());
    }

    int outputStatus = AndroidBitmap_unlockPixels(env, output);
    int sourceStatus = AndroidBitmap_unlockPixels(env, input);
    return outputStatus < 0 || sourceStatus < 0 ? nullptr : output;
}
