#include "PhotoEffects.h"
#include <vector>

namespace photo {
namespace {

Rgb encode(Rgb color) {
    const float y = .299f * color.r + .587f * color.g + .114f * color.b;
    return {y, .492f * (color.b - y), .877f * (color.r - y)};
}

Rgb decode(Rgb signal) {
    return {signal.r + 1.140f * signal.b,
            signal.r - .395f * signal.g - .581f * signal.b,
            signal.r + 2.033f * signal.g};
}

// Virtual horizontal sampling keeps carrier patterns consistent across output resolutions.
struct Scanline {
    const Image &image;
    int samples;
    float step;
    float lineStep;

    Scanline(const Image &source, float resolution) :
            image(source), samples(int(resolution)), step(float(source.width) / samples),
            lineStep(std::max(1.f, source.height / 576.f)) {}

    Rgb read(float x, float y) const {
        return encode(image.sample((x + .5f) * step - .5f, y));
    }
};

constexpr float carrierCos[] = {1.f, 0.f, -1.f, 0.f};
constexpr float carrierSin[] = {0.f, 1.f, 0.f, -1.f};

}

void palComposite(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const Scanline scan(image, p[4]);
    const int chromaRadius = int(p[1]);
    const float phaseCos = std::cos(p[3] * pi / 180.f);
    const float phaseSin = std::sin(p[3] * pi / 180.f);
    const int padding = chromaRadius * 2;
    std::vector<float> signal(scan.samples + padding * 2), previous(signal.size());
    std::vector<Rgb> decoded(scan.samples);
    int lastLine = -1;

    auto modulate = [&](int line, std::vector<float> &row) {
        const float sign = line & 1 ? -1.f : 1.f;
        for (int x = -padding; x < scan.samples + padding; ++x) {
            const auto yuv = scan.read(float(x), line * scan.lineStep);
            const float u = yuv.g * phaseCos - sign * yuv.b * phaseSin;
            const float v = yuv.g * phaseSin + sign * yuv.b * phaseCos;
            row[x + padding] = yuv.r + u * carrierCos[x & 3] + v * carrierSin[x & 3] +
                               (random(x, line, seed) - .5f) * p[2];
        }
    };

    render(image, output, amount, [&](int x, int y, Rgb) {
        const int line = int(y / scan.lineStep);
        if (line != lastLine) {
            if (line == lastLine + 1 && lastLine >= 0) {
                previous.swap(signal);
            } else {
                modulate(std::max(0, line - 1), previous);
            }
            modulate(line, signal);
            lastLine = line;

            const float sign = line & 1 ? -1.f : 1.f;
            for (int i = 0; i < scan.samples; ++i) {
                float ySum = 0.f;
                float u = 0.f, v = 0.f, delayedU = 0.f, delayedV = 0.f;
                const int count = 4 * chromaRadius;
                for (int tap = -count / 2; tap < count / 2; ++tap) {
                    const int j = i + tap + padding;
                    const int phase = (i + tap) & 3;
                    u += signal[j] * carrierCos[phase];
                    v += signal[j] * carrierSin[phase];
                    delayedU += previous[j] * carrierCos[phase];
                    delayedV += previous[j] * carrierSin[phase];
                }
                for (int tap = -1; tap <= 2; ++tap) {
                    ySum += signal[i + tap + padding];
                }
                const float delay = line > 0 ? p[5] * .5f : 0.f;
                const float luminance = mix(signal[i + padding], ySum * .25f, p[0]);
                decoded[i] = decode({luminance, 2.f / count * mix(u, delayedU, delay),
                                     sign * 2.f / count * mix(v, -delayedV, delay)});
            }
        }
        const float index = std::clamp((x + .5f) / scan.step - .5f, 0.f, float(scan.samples - 1));
        const int left = int(index);
        return mix(decoded[left], decoded[std::min(left + 1, scan.samples - 1)], index - left);
    });
}

void secam(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const Scanline scan(image, 720.f);
    std::vector<float> encoded(scan.samples), decoded(scan.samples), previous(scan.samples);
    int lastLine = -1;
    const int radius = int(p[0]);
    auto demodulate = [&](int line, std::vector<float> &chroma) {
        const bool red = line & 1;
        const float carrier = red ? 1.8f : 1.6f;
        const float deviation = p[1];
        float phase = 0.f, lastPhase = 0.f, lastValue = 0.f;
        for (int x = 0; x < scan.samples; ++x) {
            const auto yuv = scan.read(float(x), line * scan.lineStep);
            const float value = red ? yuv.b : yuv.g;
            phase += carrier + deviation * (value + p[3] * (value - lastValue));
            lastValue = value;
            const float noisyPhase = phase + (random(x, line, seed) - .5f) * p[2];
            encoded[x] = x == 0 ? value : (std::remainder(noisyPhase - lastPhase, 2.f * pi) - carrier) / deviation;
            lastPhase = noisyPhase;
        }
        float sum = 0.f;
        for (int tap = -radius; tap <= radius; ++tap) {
            sum += encoded[std::clamp(tap, 0, scan.samples - 1)];
        }
        for (int x = 0; x < scan.samples; ++x) {
            const float filtered = sum / (2 * radius + 1);
            chroma[x] = x == 0 ? filtered : mix(filtered, chroma[x - 1], p[3] * .5f);
            sum += encoded[std::min(x + radius + 1, scan.samples - 1)] -
                   encoded[std::max(x - radius, 0)];
        }
    };
    render(image, output, amount, [&](int x, int y, Rgb original) {
        const int line = int(y / scan.lineStep);
        if (lastLine != line) {
            if (lastLine >= 0 && line == lastLine + 1) previous.swap(decoded);
            else demodulate(line == 0 ? 1 : line - 1, previous);
            demodulate(line, decoded);
            lastLine = line;
        }
        const int i = std::min(int(x / scan.step), scan.samples - 1);
        const auto yuv = encode(original);
        const float error = p[4];
        const float u = (line & 1) ? mix(previous[i], decoded[i], error) : decoded[i];
        const float v = (line & 1) ? decoded[i] : mix(previous[i], decoded[i], error);
        return decode({yuv.r, u, v});
    });
}

void rfReception(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    const float scale = image.scale();
    const float delay = std::max(1.f, p[0] * scale);
    const float first = p[1], second = -.55f * first;
    const float angle = .74f * pi;
    const float phaseCos = std::cos(angle), phaseSin = std::sin(angle);
    render(image, output, amount, [&](int x, int y, Rgb original) {
        const auto yuv = encode(original);
        auto echo = encode(image.sample(x - delay, float(y)));
        const auto far = encode(image.sample(x - 2.3f * delay, float(y)));
        echo = {echo.r, echo.g * phaseCos - echo.b * phaseSin,
                echo.b * phaseCos + echo.g * phaseSin};
        const float jitter = (random(int(x / std::max(scale, 1.f)), y, seed) - .5f) * p[2];
        const float carrier = std::sin(2.f * pi * (p[3] * x / image.width + 12.f * y / image.height));
        const auto received = (yuv + echo * first + far * second) * (1.f / (1.f + first + second));
        return decode(received + Rgb{jitter + .07f * first * carrier, .035f * carrier, jitter * .4f});
    });
}

void sstvReception(const Image &image, const Output &output, const Params &p, uint32_t seed, float amount) {
    constexpr int lines = 256;
    constexpr int columns = 320;
    constexpr int oversampling = 4;
    constexpr int segmentLength = columns * oversampling;
    constexpr float sampleRate = 9600.f;
    constexpr float baseFrequency = 1500.f;
    constexpr float frequencyRange = 800.f;
    const std::array<int, 3> channelOrder = p[0] >= .5f
            ? std::array<int, 3>{0, 1, 2} : std::array<int, 3>{1, 2, 0};
    std::array<float, 3 * segmentLength> phase{};
    std::array<std::array<float, columns>, 3> decoded{};
    int lastLine = -1;

    render(image, output, amount, [&](int x, int y, Rgb) {
        const int line = y * int64_t(lines) / image.height;
        if (line != lastLine) {
            lastLine = line;
            const bool dropped = random(1, line, seed) < p[4];
            const float row = (std::max(0, line - int(dropped)) + .5f) * image.height / lines - .5f;
            float carrier = 0.f;
            for (int segment = 0; segment < 3; ++segment) {
                for (int i = 0; i < segmentLength; ++i) {
                    const int t = segment * segmentLength + i;
                    const auto rgb = image.sample((i / float(oversampling) + .5f) * image.width / columns - .5f, row);
                    const float channels[] = {rgb.r, rgb.g, rgb.b};
                    const float drift = p[1] * frequencyRange * std::sin(line * .029f + t * .00035f);
                    const float frequency = baseFrequency + channels[channelOrder[segment]] * frequencyRange + drift;
                    carrier = std::remainder(carrier + 2.f * pi * frequency / sampleRate, 2.f * pi);
                    phase[t] = carrier + (random(t, line, seed + 1) - .5f) * p[3];
                }
            }
            const float jitter = random(0, line, seed) - .5f;
            for (int segment = 0; segment < 3; ++segment) {
                const int start = segment * segmentLength;
                const int sync = int(p[2] * segmentLength * (jitter + .7f * segment - .5f));
                for (int i = 0; i < columns; ++i) {
                    float frequency = 0.f;
                    for (int tap = 0; tap < oversampling; ++tap) {
                        const int t = std::clamp(start + i * oversampling + tap + sync, 1, int(phase.size()) - 1);
                        frequency += std::remainder(phase[t] - phase[t - 1], 2.f * pi);
                    }
                    frequency *= sampleRate / (2.f * pi * oversampling);
                    decoded[channelOrder[segment]][i] = (frequency - baseFrequency) / frequencyRange;
                }
            }
        }
        const int i = int(int64_t(x) * columns / image.width);
        return Rgb{decoded[0][i], decoded[1][i], decoded[2][i]};
    });
}

void lcdSubpixels(const Image &image, const Output &output, const Params &p, uint32_t, float amount) {
    const float cell = std::max(3.f, p[0] * image.scale());
    const float edge = std::min(.2f, .75f / cell);
    const float angle = p[3] * pi / 180.f;
    const float squeeze = std::max(.35f, std::cos(angle));
    const float columnWidth = std::max(3.f, cell * squeeze);
    render(image, output, amount, [&](int x, int y, Rgb) {
        const float u = (x + .5f) / columnWidth;
        const float v = (y + .5f) / cell;
        auto color = image.sample((std::floor(u) + .5f) * columnWidth - .5f,
                                  (std::floor(v) + .5f) * cell - .5f);
        const float px = fraction(u);
        const float py = fraction(v);
        auto aperture = [&](int channel) {
            const float center = (channel + .5f) / 3.f;
            const float radius = (1.f - p[1]) / 6.f;
            return 1.f - smooth(radius - edge, radius + edge, std::abs(px - center));
        };
        const float vertical = smooth(p[1] / 2.f - edge, p[1] / 2.f + edge, std::min(py, 1.f - py));
        const bool bgr = p[2] >= .5f;
        return Rgb{color.r * aperture(bgr ? 2 : 0) * (1.f - .24f * std::sin(angle)),
                   color.g * aperture(1),
                   color.b * aperture(bgr ? 0 : 2) * (1.f + .24f * std::sin(angle))} * (vertical * 1.8f);
    });
}

}
