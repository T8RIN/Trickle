#pragma once

#include <android/bitmap.h>
#include <limits>
#include <new>
#include "BitmapUtils.h"
#include "PhotoEffects.h"

namespace photo {

class LockedBitmap {
public:
    LockedBitmap(JNIEnv *env, jobject bitmap) : env(env), bitmap(bitmap) {
        if (AndroidBitmap_lockPixels(env, bitmap, &pixels) != ANDROID_BITMAP_RESULT_SUCCESS) {
            pixels = nullptr;
        }
    }

    ~LockedBitmap() {
        if (pixels) AndroidBitmap_unlockPixels(env, bitmap);
    }

    LockedBitmap(const LockedBitmap &) = delete;
    LockedBitmap &operator=(const LockedBitmap &) = delete;

    uint8_t *data() const { return static_cast<uint8_t *>(pixels); }

private:
    JNIEnv *env;
    jobject bitmap;
    void *pixels = nullptr;
};

template<typename F>
jobject apply(JNIEnv *env, jobject input, F filter) {
    const EncodingInterruption interruption(env);
    if (interruption.throwIfRequested()) return nullptr;

    AndroidBitmapInfo sourceInfo{}, targetInfo{};
    if (AndroidBitmap_getInfo(env, input, &sourceInfo) != ANDROID_BITMAP_RESULT_SUCCESS ||
        sourceInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888 || sourceInfo.width == 0 ||
        sourceInfo.height == 0 || sourceInfo.width > std::numeric_limits<int>::max() ||
        sourceInfo.height > std::numeric_limits<int>::max()) return nullptr;

    auto result = createBitmap(env, sourceInfo.width, sourceInfo.height);
    if (!result || env->ExceptionCheck() ||
        AndroidBitmap_getInfo(env, result, &targetInfo) != ANDROID_BITMAP_RESULT_SUCCESS) return nullptr;

    try {
        const LockedBitmap source(env, input);
        if (!source.data()) return nullptr;
        const LockedBitmap target(env, result);
        if (!target.data()) return nullptr;
        const photo::Image image{int(sourceInfo.width), int(sourceInfo.height), sourceInfo.stride, source.data(),
                                 (sourceInfo.flags & ANDROID_BITMAP_FLAGS_ALPHA_MASK) == ANDROID_BITMAP_FLAGS_ALPHA_PREMUL};
        const photo::Output output{targetInfo.stride, target.data()};
        filter(image, output);
    } catch (const std::bad_alloc &) {
        if (!interruption.throwIfRequested()) {
            if (const auto error = env->FindClass("java/lang/OutOfMemoryError")) {
                env->ThrowNew(error, "Cannot allocate filter workspace");
            }
        }
        return nullptr;
    }

    return interruption.throwIfRequested() ? nullptr : result;
}

}
