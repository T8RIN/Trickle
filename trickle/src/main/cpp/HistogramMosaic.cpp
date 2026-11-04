#include <vector>
#include "PhotoFilter.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_histogramMosaicImpl(
        JNIEnv *env, jobject, jobject input, jint cells, jint bins, jfloat gap, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        using namespace photo;
        const float cell = float(std::max(image.width, image.height)) / cells;
        const int columns = std::max(1, int(std::ceil(image.width / cell)));
        const int rows = std::max(1, int(std::ceil(image.height / cell)));
        struct Bin { float count = 0; Rgb color{}; };
        std::vector<Bin> histogram(size_t(columns) * rows * bins);
        for (int y = 0; y < image.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < image.width; ++x) {
                if ((x & 2047) == 0 && EncodingInterruption::currentRequested()) return;
                const auto color = image.read(x, y);
                const int index = std::min(bins - 1, int(luma(color) * bins));
                auto &bin = histogram[(size_t(int(y / cell)) * columns + int(x / cell)) * bins + index];
                const float weight = image.pixel(x, y)[3] / 255.f;
                bin.count += weight;
                bin.color = bin.color + color * weight;
            }
        }
        std::vector<float> peaks(size_t(columns) * rows, 1.f);
        for (size_t i = 0; i < histogram.size(); ++i) {
            auto &bin = histogram[i];
            peaks[i / bins] = std::max(peaks[i / bins], bin.count);
            if (bin.count > 0) bin.color = bin.color * (1.f / bin.count);
        }
        render(image, output, strength, [&](int x, int y, Rgb) {
            const int cx = int(x / cell), cy = int(y / cell);
            const float u = (x + .5f) / cell - cx, v = (y + .5f) / cell - cy;
            const float inner = std::max(.01f, 1.f - 2.f * gap);
            const float bar = (u - gap) / inner * bins;
            const int index = std::clamp(int(bar), 0, bins - 1);
            const size_t tile = size_t(cy) * columns + cx;
            const auto &bin = histogram[tile * bins + index];
            const float height = bin.count / peaks[tile] * inner;
            const float aa = std::min(.1f, 1.f / std::max(1.f, cell));
            const float sides = smooth(0.f, aa * bins, fraction(bar)) *
                                (1.f - smooth(1.f - aa * bins, 1.f, fraction(bar)));
            const float mask = smooth(1.f - gap - height - aa, 1.f - gap - height + aa, v) *
                               (1.f - smooth(1.f - gap - aa, 1.f - gap + aa, v));
            const float inside = (u >= gap && u <= 1.f - gap && bin.count > 0) ? 1.f : 0.f;
            return mix(gray(.045f), bin.color * .8f + gray(.2f), unit(mask * sides * inside));
        });
    });
}
