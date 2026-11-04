#include "PhotoFilter.h"
#include "AnalysisImage.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_localEntropyMapImpl(
        JNIEnv *env, jobject, jobject input, jfloat radius, jint bins, jfloat gain, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        AnalysisImage source(image, 768);
        if (EncodingInterruption::currentRequested()) return;
        const int width = source.width, height = source.height;
        const int r = std::max(1, int(std::round(radius * std::max(width, height) / 1024.f)));
        std::vector<uint8_t> quantized(size_t(width) * height);
        std::vector<float> entropy(quantized.size());
        std::vector<std::array<int, 32>> columns(width);
        for (size_t i = 0; i < quantized.size(); ++i) {
            if ((i & 16383) == 0 && EncodingInterruption::currentRequested()) return;
            quantized[i] = uint8_t(std::min(bins - 1, int(source.pixels[i] * bins)));
        }
        auto changeRow = [&](int y, int delta) {
            for (int x = 0; x < width; ++x) columns[x][quantized[size_t(y) * width + x]] += delta;
        };
        for (int y = 0; y <= std::min(r, height - 1); ++y) changeRow(y, 1);
        const float maximum = std::log2(float(bins));
        for (int y = 0; y < height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            if (y > 0) {
                if (y - r - 1 >= 0) changeRow(y - r - 1, -1);
                if (y + r < height) changeRow(y + r, 1);
            }
            std::array<int, 32> histogram{};
            auto changeColumn = [&](int x, int delta) {
                for (int b = 0; b < bins; ++b) histogram[b] += delta * columns[x][b];
            };
            for (int x = 0; x <= std::min(r, width - 1); ++x) changeColumn(x, 1);
            const int rowCount = std::min(height - 1, y + r) - std::max(0, y - r) + 1;
            for (int x = 0; x < width; ++x) {
                if (x > 0) {
                    if (x - r - 1 >= 0) changeColumn(x - r - 1, -1);
                    if (x + r < width) changeColumn(x + r, 1);
                }
                const int columnCount = std::min(width - 1, x + r) - std::max(0, x - r) + 1;
                const float area = float(rowCount * columnCount);
                float value = 0;
                for (int b = 0; b < bins; ++b) {
                    if (histogram[b] == 0) continue;
                    const float p = histogram[b] / area;
                    value -= p * std::log2(p);
                }
                entropy[size_t(y) * width + x] = gain * value / maximum;
            }
        }
        source.pixels.swap(entropy);
        render(image, output, strength, [&](int x, int y, Rgb) {
            return gray(source.sample((x + .5f) * width / image.width - .5f,
                                      (y + .5f) * height / image.height - .5f));
        });
    });
}
