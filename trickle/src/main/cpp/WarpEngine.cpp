#include <jni.h>
#include <android/bitmap.h>
#include <cmath>
#include <vector>

enum WarpMode {
    MOVE = 0,
    GROW = 1,
    SHRINK = 2,
    SWIRL_CW = 3,
    SWIRL_CCW = 4
};

struct WarpBrush {
    float radius;
    float hardness;
    float strength;
};

struct DisplacementMap {
    int w, h;
    std::vector<float> dx;
    std::vector<float> dy;

    DisplacementMap(int w_, int h_)
            : w(w_), h(h_), dx(w_ * h_, 0.f), dy(w_ * h_, 0.f) {}

    inline int index(int x, int y) const {
        return y * w + x;
    }
};

static inline float smoothstep(float edge0, float edge1, float x) {
    float t = (x - edge0) / (edge1 - edge0);
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    return t * t * (3.f - 2.f * t);
}

static inline int lerpColor(int a, int b, float t) {
    int ar = (a >> 16) & 0xff;
    int ag = (a >> 8) & 0xff;
    int ab = a & 0xff;

    int br = (b >> 16) & 0xff;
    int bg = (b >> 8) & 0xff;
    int bb = b & 0xff;

    int r = ar + (int)((br - ar) * t);
    int g = ag + (int)((bg - ag) * t);
    int bl = ab + (int)((bb - ab) * t);

    return 0xff000000 | (r << 16) | (g << 8) | bl;
}

struct WarpEngine {
    int w, h;
    std::vector<int> pixels;
    DisplacementMap map;
    bool hasLast = false;
    float lastX = 0.f;
    float lastY = 0.f;

    WarpEngine(AndroidBitmapInfo& info, void* ptr)
            : w(info.width),
              h(info.height),
              pixels(w * h),
              map(w, h) {
        memcpy(pixels.data(), ptr, w * h * 4);
    }

    void applyStroke(
            float fromX,
            float fromY,
            float toX,
            float toY,
            const WarpBrush& brush,
            WarpMode mode
    ) {
        if (!hasLast) {
            lastX = fromX;
            lastY = fromY;
            hasLast = true;
        }

        interpolateAndApplyStroke(
                lastX, lastY, toX, toY, brush, mode
        );

        lastX = toX;
        lastY = toY;
    }

    void interpolateAndApplyStroke(
            float fromX,
            float fromY,
            float toX,
            float toY,
            const WarpBrush& brush,
            WarpMode mode
    ) {
        float dx = toX - fromX;
        float dy = toY - fromY;
        float distance = std::sqrt(dx * dx + dy * dy);
        float step = brush.radius / 2.f;

        if (distance <= step) {
            applyStrokeToPoint(fromX, fromY, toX, toY, brush, mode);
        } else {
            int steps = (int)std::ceil(distance / step);
            float stepX = dx / steps;
            float stepY = dy / steps;

            for (int i = 0; i < steps; i++) {
                float fx = fromX + stepX * std::max(i - 1, 0);
                float fy = fromY + stepY * std::max(i - 1, 0);
                float tx = fromX + stepX * i;
                float ty = fromY + stepY * i;
                applyStrokeToPoint(fx, fy, tx, ty, brush, mode);
            }
        }
    }

    void applyStrokeToPoint(
            float fromX,
            float fromY,
            float toX,
            float toY,
            const WarpBrush& brush,
            WarpMode mode
    ) {
        float r = brush.radius;
        float r2 = r * r;

        int minX = std::max(0, (int)(toX - r));
        int maxX = std::min(w - 1, (int)(toX + r));
        int minY = std::max(0, (int)(toY - r));
        int maxY = std::min(h - 1, (int)(toY + r));

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                float dx = x - toX;
                float dy = y - toY;
                float dist2 = dx * dx + dy * dy;
                if (dist2 > r2) continue;

                float dist = std::sqrt(dist2);
                float t = 1.f - dist / r;

                float amp = 1.f;
                if (mode == GROW || mode == SHRINK) amp = 0.03f;
                else if (mode == SWIRL_CW || mode == SWIRL_CCW) amp = 0.2f;

                float falloff =
                        smoothstep(brush.hardness, 1.f, t) *
                                (brush.strength * amp);

                int idx = map.index(x, y);

                switch (mode) {
                    case MOVE:
                        map.dx[idx] += (fromX - toX) * falloff;
                        map.dy[idx] += (fromY - toY) * falloff;
                        break;

                    case GROW:
                    case SHRINK: {
                        float len = std::sqrt(dx * dx + dy * dy);
                        if (len > 0.f) {
                            float sign = (mode == GROW) ? -1.f : 1.f;
                            map.dx[idx] += sign * (dx / len) * falloff * r;
                            map.dy[idx] += sign * (dy / len) * falloff * r;
                        }
                        break;
                    }

                    case SWIRL_CW:
                    case SWIRL_CCW: {
                        float angleMax = 0.8f;
                        float angle = t * angleMax * falloff *
                                (mode == SWIRL_CW ? 1.f : -1.f);
                        float s = std::sin(angle);
                        float c = std::cos(angle);

                        float rx = dx * c - dy * s;
                        float ry = dx * s + dy * c;

                        map.dx[idx] += (rx - dx);
                        map.dy[idx] += (ry - dy);
                        break;
                    }
                }
            }
        }
    }

    inline int sampleBilinear(float fx, float fy) {
        int x0 = std::min(w - 1, std::max(0, (int)fx));
        int y0 = std::min(h - 1, std::max(0, (int)fy));
        int x1 = std::min(w - 1, x0 + 1);
        int y1 = std::min(h - 1, y0 + 1);

        float dx = fx - x0;
        float dy = fy - y0;

        int c00 = pixels[y0 * w + x0];
        int c10 = pixels[y0 * w + x1];
        int c01 = pixels[y1 * w + x0];
        int c11 = pixels[y1 * w + x1];

        return lerpColor(
                lerpColor(c00, c10, dx),
                lerpColor(c01, c11, dx),
                dy
        );
    }

    void render(void* outPtr) {
        int* out = (int*)outPtr;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int i = y * w + x;
                float sx = x + map.dx[i];
                float sy = y + map.dy[i];
                out[i] = sampleBilinear(sx, sy);
            }
        }
    }
};
