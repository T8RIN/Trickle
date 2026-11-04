#pragma once

#include <limits>
#include "PhotoEffects.h"

namespace photo {

struct MaterialCell {
    int x = 0;
    int y = 0;
    float centerX = 0.f;
    float centerY = 0.f;
    float edge = 0.f;
    float normalX = 0.f;
    float normalY = 0.f;
};

inline MaterialCell materialCell(float x, float y, uint32_t seed) {
    MaterialCell result;
    const int ix = int(std::floor(x)), iy = int(std::floor(y));
    float nearest = std::numeric_limits<float>::max();
    for (int cy = iy - 1; cy <= iy + 1; ++cy) {
        for (int cx = ix - 1; cx <= ix + 1; ++cx) {
            const float px = cx + .15f + .7f * random(cx, cy, seed);
            const float py = cy + .15f + .7f * random(cx, cy, seed ^ 0xacf32u);
            const float distance = (x - px) * (x - px) + (y - py) * (y - py);
            if (distance < nearest) {
                nearest = distance;
                result.x = cx;
                result.y = cy;
                result.centerX = px;
                result.centerY = py;
            }
        }
    }
    result.edge = std::numeric_limits<float>::max();
    for (int cy = result.y - 2; cy <= result.y + 2; ++cy) {
        for (int cx = result.x - 2; cx <= result.x + 2; ++cx) {
            if (cx == result.x && cy == result.y) continue;
            const float px = cx + .15f + .7f * random(cx, cy, seed);
            const float py = cy + .15f + .7f * random(cx, cy, seed ^ 0xacf32u);
            const float dx = result.centerX - px, dy = result.centerY - py;
            const float separation = std::sqrt(dx * dx + dy * dy);
            const float distance = ((x - px) * (x - px) + (y - py) * (y - py) - nearest) / (2.f * separation);
            if (distance < result.edge) {
                result.edge = std::max(0.f, distance);
                result.normalX = dx / separation;
                result.normalY = dy / separation;
            }
        }
    }
    return result;
}

}
