#include "PhotoFilter.h"
#include <limits>
#include <vector>

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_foldedBookImpl(
        JNIEnv *env, jobject, jobject input, jint pages, jfloat depth, jfloat viewAngle,
        jint paper, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const int count = std::clamp(int(pages), 24, 256);
        const float spacing = float(image.width) / count;
        const float depthScale = std::clamp(depth, 0.f, 128.f) * image.scale();
        const float angle = std::clamp(viewAngle, -60.f, 60.f) * photo::pi / 180.f;
        const float sine = std::sin(angle), cosine = std::cos(angle);
        const float offset = image.width * (1.f - cosine) * .5f - sine * depthScale * .5f;
        const photo::Rgb paperColor = {((uint32_t(paper) >> 16) & 255) / 255.f,
                                       ((uint32_t(paper) >> 8) & 255) / 255.f, (uint32_t(paper) & 255) / 255.f};
        std::vector<float> zbuffer(image.width), heights(count), tones(count);
        std::vector<photo::Rgb> rowColor(image.width);
        for (int y = 0; y < image.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            std::fill(zbuffer.begin(), zbuffer.end(), -std::numeric_limits<float>::infinity());
            std::fill(rowColor.begin(), rowColor.end(), paperColor * .28f);
            for (int page = 0; page < count; ++page) {
                const float x = (page + .5f) * spacing - .5f;
                tones[page] = photo::luma(image.sample(x, float(y)));
                heights[page] = depthScale * (.08f + .92f * tones[page]);
            }
            for (int page = 0; page < count; ++page) {
                const float worldX = (page + .5f) * spacing - .5f;
                const float back = cosine * worldX + offset;
                const float front = back + sine * heights[page];
                const float rimWidth = std::max(.3f, spacing * .18f);
                const int begin = std::max(0, int(std::floor(std::min(back, front) - rimWidth - 1.f)));
                const int end = std::min(image.width - 1, int(std::ceil(std::max(back, front) + rimWidth + 1.f)));
                const float neighbor = std::max(heights[std::max(0, page - 1)], heights[std::min(count - 1, page + 1)]);
                const float recess = std::max(0.f, neighbor - heights[page]);
                const float ambient = .45f + .55f * std::exp(-recess / std::max(1.f, spacing * 2.f));
                for (int x = begin; x <= end; ++x) {
                    const float distance = std::abs(x - front);
                    const float rim = photo::smooth(-.65f, .65f, distance + rimWidth) -
                                      photo::smooth(-.65f, .65f, distance - rimWidth);
                    float z = std::abs(sine) > .0001f ? (x - back) / sine : heights[page];
                    const bool onFace = z >= 0.f && z <= heights[page] && std::abs(sine) > .0001f;
                    if (!onFace && rim <= 0.f) continue;
                    z = std::clamp(z, 0.f, heights[page]);
                    const float cameraDepth = cosine * z - sine * worldX;
                    if (cameraDepth < zbuffer[x]) continue;
                    zbuffer[x] = cameraDepth;
                    const float side = ambient * (.3f + .42f * photo::unit(z / std::max(depthScale, .001f)));
                    const float edge = ambient * (.6f + .4f * tones[page]);
                    rowColor[x] = photo::mix(paperColor * side, paperColor * edge, rim);
                }
            }
            for (int x = 0; x < image.width; ++x) {
                if ((x & 2047) == 0 && EncodingInterruption::currentRequested()) return;
                output.write(x, y, photo::mix(image.read(x, y), rowColor[x], photo::unit(strength)), image.pixel(x, y)[3]);
            }
        }
    });
}
