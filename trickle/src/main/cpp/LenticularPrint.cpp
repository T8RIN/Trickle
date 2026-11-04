#include "PhotoFilter.h"

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_PhotoEffectsPipelineImpl_lenticularPrintImpl(
        JNIEnv *env, jobject, jobject input, jfloat lensWidth, jfloat viewAngle, jfloat refraction,
        jfloat gloss, jfloat rotation, jfloat strength) {
    return photo::apply(env, input, [&](const photo::Image &image, const photo::Output &output) {
        const float pitch = std::max(.01f, std::clamp(lensWidth, 4.f, 64.f) * image.scale());
        const float view = std::clamp(viewAngle, -50.f, 50.f) * photo::pi / 180.f;
        const float angle = std::clamp(rotation, -90.f, 90.f) * photo::pi / 180.f;
        const float cosine = std::cos(angle), sine = std::sin(angle);
        const float eta = 1.f / std::clamp(refraction, 1.f, 2.2f);
        const float glossiness = photo::unit(gloss), amount = photo::unit(strength);
        const float incidentX = std::sin(view), incidentZ = -std::cos(view);
        const float halfX = std::sin((view - .6f) * .5f), halfZ = std::cos((view - .6f) * .5f);
        for (int y = 0; y < image.height; ++y) {
            if (EncodingInterruption::currentRequested()) return;
            for (int x = 0; x < image.width; ++x) {
                if ((x & 1023) == 0 && EncodingInterruption::currentRequested()) return;
                const float u = (x + .5f) * cosine + (y + .5f) * sine;
                const float nx = 2.f * photo::fraction(u / pitch) - 1.f;
                const float nz = std::sqrt(std::max(.0001f, 1.f - nx * nx));
                const float incidentCosine = std::max(.001f, -(nx * incidentX + nz * incidentZ));
                const float k = std::max(0.f, 1.f - eta * eta * (1.f - incidentCosine * incidentCosine));
                const float normalScale = eta * incidentCosine - std::sqrt(k);
                const float transmittedX = eta * incidentX + normalScale * nx;
                const float transmittedZ = eta * incidentZ + normalScale * nz;
                const float thickness = pitch * .5f * (nz + .18f);
                const float displacement = std::clamp(-transmittedX * thickness / std::min(-.05f, transmittedZ), -pitch, pitch);
                const float px = std::clamp(x + displacement * cosine, 0.f, float(image.width - 1));
                const float py = std::clamp(y + displacement * sine, 0.f, float(image.height - 1));
                const int ix = int(px), iy = int(py);
                const float fx = px - ix, fy = py - iy;
                photo::Rgb associated{};
                float alpha = 0.f;
                for (int row = 0; row < 2; ++row) {
                    for (int col = 0; col < 2; ++col) {
                        const int sx = std::min(image.width - 1, ix + col), sy = std::min(image.height - 1, iy + row);
                        const float weight = (col ? fx : 1.f - fx) * (row ? fy : 1.f - fy);
                        const float coverage = image.pixel(sx, sy)[3] / 255.f * weight;
                        associated = associated + image.read(sx, sy) * coverage;
                        alpha += coverage;
                    }
                }
                const float fresnel = .04f + .96f * std::pow(1.f - photo::unit(incidentCosine), 5.f);
                const float highlight = std::pow(std::max(0.f, nx * halfX + nz * halfZ), 48.f);
                const float reflection = glossiness * (.25f * fresnel + .4f * highlight);
                associated = photo::mix(associated, photo::gray(alpha), reflection);
                const float originalAlpha = image.pixel(x, y)[3] / 255.f;
                const float resultAlpha = photo::mix(originalAlpha, alpha, amount);
                const auto result = photo::mix(image.read(x, y) * originalAlpha, associated, amount);
                output.write(x, y, result * (resultAlpha > 0.f ? 1.f / resultAlpha : 0.f), uint8_t(resultAlpha * 255.f + .5f));
            }
        }
    });
}
