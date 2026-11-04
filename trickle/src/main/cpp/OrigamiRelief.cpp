#include "PhotoFilter.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_origamiReliefImpl(
        JNIEnv *env, jobject, jobject input, jfloat cellSize, jfloat foldHeight,
        jfloat lightAngle, jfloat crease, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const float cell = std::max(.01f, std::clamp(cellSize, 16.f, 160.f) * image.scale());
        const float height = photo::unit(foldHeight);
        const float angle = lightAngle * photo::pi / 180.f;
        const float lx = std::cos(angle) * .6f, ly = std::sin(angle) * .6f, lz = .8f;
        const float line = std::max(.5f, image.scale());
        photo::render(image, output, photo::unit(strength), [&](int x, int y, photo::Rgb original) {
            const float u = (x + .5f) / cell, v = (y + .5f) / cell;
            const int col = int(std::floor(u)), row = int(std::floor(v));
            const float a = photo::fraction(u), b = photo::fraction(v);
            const float centerTone = photo::luma(image.sample((col + .5f) * cell - .5f,
                                                              (row + .5f) * cell - .5f));
            const float apex = height * (.6f + .4f * centerTone) * ((col + row) & 1 ? -1.f : 1.f);
            // Four triangles meet at a raised or recessed center. All shared vertices stay at zero.
            const float distances[] = {a, 1.f - a, b, 1.f - b};
            const int face = int(std::min_element(distances, distances + 4) - distances);
            const float dx = face == 0 ? 2.f * apex : face == 1 ? -2.f * apex : 0.f;
            const float dy = face == 2 ? 2.f * apex : face == 3 ? -2.f * apex : 0.f;
            const float normalLength = std::sqrt(1.f + dx * dx + dy * dy);
            const float diffuse = std::max(0.f, (lz - dx * lx - dy * ly) / normalLength);
            const float lighting = (.38f + .78f * diffuse) / (.38f + .78f * lz);
            const float seamDistance = std::min({distances[face], std::abs(a - b) * .70710678f,
                                                  std::abs(a + b - 1.f) * .70710678f}) * cell;
            const float seam = std::exp(-seamDistance * seamDistance / (line * line));
            const float shading = lighting * (1.f - .3f * photo::unit(crease) * height * seam);
            return original * shading;
        });
    });
}
