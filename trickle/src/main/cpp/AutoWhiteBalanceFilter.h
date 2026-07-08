/*
 * AutoWhiteBalanceFilter.h
 */

#ifndef AUTOWHITEBALANCEFILTER_H_
#define AUTOWHITEBALANCEFILTER_H_

#include "ImageFilter.h"

struct AutoWhiteBalanceOptions {
    float strength;
    float clipPercent;

    AutoWhiteBalanceOptions(float strength, float clipPercent) :
            strength(strength), clipPercent(clipPercent) {
    }
};

class AutoWhiteBalanceFilter : public ImageFilter {
public:
    AutoWhiteBalanceFilter(int *pixels, int width, int height, AutoWhiteBalanceOptions options);

    int *procImage() override;

private:
    AutoWhiteBalanceOptions options;
};

#endif /* AUTOWHITEBALANCEFILTER_H_ */
