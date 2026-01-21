#include <jni.h>
#include <android/bitmap.h>
#include <vector>
#include <cmath>
#include <algorithm>

extern "C"
JNIEXPORT jint JNICALL
Java_com_t8rin_trickle_TrickleUtils_calculateBrightnessEstimate(
        JNIEnv *env,
        jobject /* this */,
        jobject bitmap,
        jint pixelSpacing
) {
    AndroidBitmapInfo info;
    void *pixelsPtr;

    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixelsPtr);

    int width = info.width;
    int height = info.height;
    int size = width * height;

    uint32_t *pixels = static_cast<uint32_t *>(pixelsPtr);

    long r = 0;
    long g = 0;
    long b = 0;
    long n = 0;

    for (int i = 0; i < size; i += pixelSpacing) {
        uint32_t color = pixels[i];
        r += (color >> 16) & 0xFF; // red
        g += (color >> 8) & 0xFF; // green
        b += color & 0xFF;         // blue
        n++;
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    return (jint) ((r + g + b) / (n * 3));
}

static inline int clamp(int v, int minv, int maxv) {
    return std::max(minv, std::min(v, maxv));
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_TrickleUtils_fastBlurImpl(
        JNIEnv *env,
        jobject /* this */,
        jobject srcBitmap,
        jfloat scale,
        jint radius
) {
    if (srcBitmap == nullptr || radius < 1) return srcBitmap;

    AndroidBitmapInfo srcInfo;
    if (AndroidBitmap_getInfo(env, srcBitmap, &srcInfo) != ANDROID_BITMAP_RESULT_SUCCESS)
        return srcBitmap;

    const int scaledW = std::max(1, (int) std::round(srcInfo.width * scale));
    const int scaledH = std::max(1, (int) std::round(srcInfo.height * scale));

    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jclass configCls = env->FindClass("android/graphics/Bitmap$Config");

    jmethodID createScaled =
            env->GetStaticMethodID(bitmapCls, "createScaledBitmap",
                    "(Landroid/graphics/Bitmap;IIZ)Landroid/graphics/Bitmap;");
    jobject scaledBitmap =
            env->CallStaticObjectMethod(bitmapCls, createScaled,
                    srcBitmap, scaledW, scaledH, JNI_TRUE);

    jmethodID getConfig = env->GetMethodID(bitmapCls, "getConfig",
            "()Landroid/graphics/Bitmap$Config;");
    jobject config = env->CallObjectMethod(scaledBitmap, getConfig);

    jmethodID copy =
            env->GetMethodID(bitmapCls, "copy",
                    "(Landroid/graphics/Bitmap$Config;Z)Landroid/graphics/Bitmap;");
    jobject bitmap = env->CallObjectMethod(scaledBitmap, copy, config, JNI_TRUE);

    AndroidBitmapInfo info;
    void *pixelsPtr;
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixelsPtr);

    const int w = info.width;
    const int h = info.height;
    const int wm = w - 1;
    const int hm = h - 1;
    const int wh = w * h;
    const int div = radius + radius + 1;

    uint32_t *pix = static_cast<uint32_t *>(pixelsPtr);

    std::vector<int> r(wh), g(wh), b(wh);
    std::vector<int> vmin(std::max(w, h)), vmax(std::max(w, h));
    std::vector<int> dv(256 * div);

    for (int i = 0; i < 256 * div; i++)
        dv[i] = i / div;

    int yi = 0, yw = 0;

    for (int y = 0; y < h; y++) {
        int rsum = 0, gsum = 0, bsum = 0;
        for (int i = -radius; i <= radius; i++) {
            int p = pix[yi + clamp(i, 0, wm)];
            rsum += (p >> 16) & 0xFF;
            gsum += (p >> 8) & 0xFF;
            bsum += p & 0xFF;
        }
        for (int x = 0; x < w; x++) {
            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];

            if (y == 0) {
                vmin[x] = clamp(x + radius + 1, 0, wm);
                vmax[x] = clamp(x - radius, 0, wm);
            }

            int p1 = pix[yw + vmin[x]];
            int p2 = pix[yw + vmax[x]];

            rsum += ((p1 >> 16) & 0xFF) - ((p2 >> 16) & 0xFF);
            gsum += ((p1 >> 8) & 0xFF) - ((p2 >> 8) & 0xFF);
            bsum += (p1 & 0xFF) - (p2 & 0xFF);
            yi++;
        }
        yw += w;
    }

    for (int x = 0; x < w; x++) {
        int rsum = 0, gsum = 0, bsum = 0;
        int yp = -radius * w;

        for (int i = -radius; i <= radius; i++) {
            yi = clamp(yp, 0, hm * w) + x;
            rsum += r[yi];
            gsum += g[yi];
            bsum += b[yi];
            yp += w;
        }

        yi = x;
        for (int y = 0; y < h; y++) {
            pix[yi] =
                    0xFF000000 |
                            (dv[rsum] << 16) |
                            (dv[gsum] << 8) |
                            dv[bsum];

            if (x == 0) {
                vmin[y] = clamp(y + radius + 1, 0, hm) * w;
                vmax[y] = clamp(y - radius, 0, hm) * w;
            }

            int p1 = x + vmin[y];
            int p2 = x + vmax[y];

            rsum += r[p1] - r[p2];
            gsum += g[p1] - g[p2];
            bsum += b[p1] - b[p2];

            yi += w;
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    jmethodID scaleBack =
            env->GetStaticMethodID(bitmapCls, "createScaledBitmap",
                    "(Landroid/graphics/Bitmap;IIZ)Landroid/graphics/Bitmap;");

    jobject result =
            env->CallStaticObjectMethod(
                    bitmapCls,
                    scaleBack,
                    bitmap,
                    (jint) (w / scale),
                    (jint) (h / scale),
                    JNI_TRUE
            );

    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_TrickleUtils_stackBlurImpl(
        JNIEnv *env,
        jobject /* this */,
        jobject srcBitmap,
        jfloat scale,
        jint radius
) {
    if (srcBitmap == nullptr || radius < 1) return srcBitmap;

    AndroidBitmapInfo srcInfo;
    if (AndroidBitmap_getInfo(env, srcBitmap, &srcInfo) != ANDROID_BITMAP_RESULT_SUCCESS)
        return srcBitmap;

    int scaledW = std::max(1, (int) std::round(srcInfo.width * scale));
    int scaledH = std::max(1, (int) std::round(srcInfo.height * scale));

    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");

    jmethodID createScaled =
            env->GetStaticMethodID(bitmapCls, "createScaledBitmap",
                    "(Landroid/graphics/Bitmap;IIZ)Landroid/graphics/Bitmap;");
    jobject scaledBitmap =
            env->CallStaticObjectMethod(bitmapCls, createScaled,
                    srcBitmap, scaledW, scaledH, JNI_TRUE);

    jmethodID getConfig =
            env->GetMethodID(bitmapCls, "getConfig",
                    "()Landroid/graphics/Bitmap$Config;");
    jobject config = env->CallObjectMethod(scaledBitmap, getConfig);

    jmethodID copy =
            env->GetMethodID(bitmapCls, "copy",
                    "(Landroid/graphics/Bitmap$Config;Z)Landroid/graphics/Bitmap;");
    jobject bitmap = env->CallObjectMethod(scaledBitmap, copy, config, JNI_TRUE);

    AndroidBitmapInfo info;
    void *pixelsPtr = nullptr;
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixelsPtr);

    const int w = info.width;
    const int h = info.height;
    const int wm = w - 1;
    const int hm = h - 1;
    const int wh = w * h;
    const int div = radius + radius + 1;

    uint32_t *pix = static_cast<uint32_t *>(pixelsPtr);

    std::vector<int> r(wh), g(wh), b(wh);
    std::vector<int> vmin(std::max(w, h));

    int divsum = (div + 1) >> 1;
    divsum *= divsum;

    std::vector<int> dv(256 * divsum);
    for (int i = 0; i < 256 * divsum; i++)
        dv[i] = i / divsum;

    int yw = 0, yi = 0;

    std::vector<std::array<int, 3>> stack(div);

    int r1 = radius + 1;

    for (int y = 0; y < h; y++) {
        int rsum = 0, gsum = 0, bsum = 0;
        int routsum = 0, goutsum = 0, boutsum = 0;
        int rinsum = 0, ginsum = 0, binsum = 0;

        for (int i = -radius; i <= radius; i++) {
            int p = pix[yi + clamp(i, 0, wm)];
            auto &sir = stack[i + radius];

            sir[0] = (p >> 16) & 0xFF;
            sir[1] = (p >> 8) & 0xFF;
            sir[2] = p & 0xFF;

            int rbs = r1 - std::abs(i);
            rsum += sir[0] * rbs;
            gsum += sir[1] * rbs;
            bsum += sir[2] * rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
            }
        }

        int stackpointer = radius;

        for (int x = 0; x < w; x++) {
            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;

            int stackstart = stackpointer - radius + div;
            auto &sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];

            if (y == 0)
                vmin[x] = std::min(x + radius + 1, wm);

            int p = pix[yw + vmin[x]];
            sir[0] = (p >> 16) & 0xFF;
            sir[1] = (p >> 8) & 0xFF;
            sir[2] = p & 0xFF;

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;

            stackpointer = (stackpointer + 1) % div;
            auto &sir2 = stack[stackpointer];

            routsum += sir2[0];
            goutsum += sir2[1];
            boutsum += sir2[2];

            rinsum -= sir2[0];
            ginsum -= sir2[1];
            binsum -= sir2[2];

            yi++;
        }
        yw += w;
    }

    for (int x = 0; x < w; x++) {
        int rsum = 0, gsum = 0, bsum = 0;
        int routsum = 0, goutsum = 0, boutsum = 0;
        int rinsum = 0, ginsum = 0, binsum = 0;

        int yp = -radius * w;

        for (int i = -radius; i <= radius; i++) {
            yi = std::max(0, yp) + x;
            auto &sir = stack[i + radius];

            sir[0] = r[yi];
            sir[1] = g[yi];
            sir[2] = b[yi];

            int rbs = r1 - std::abs(i);
            rsum += r[yi] * rbs;
            gsum += g[yi] * rbs;
            bsum += b[yi] * rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
            }

            if (i < hm) yp += w;
        }

        yi = x;
        int stackpointer = radius;

        for (int y = 0; y < h; y++) {
            pix[yi] =
                    (pix[yi] & 0xFF000000) |
                            (dv[rsum] << 16) |
                            (dv[gsum] << 8) |
                            dv[bsum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;

            int stackstart = stackpointer - radius + div;
            auto &sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];

            if (x == 0)
                vmin[y] = std::min(y + r1, hm) * w;

            int p = x + vmin[y];
            sir[0] = r[p];
            sir[1] = g[p];
            sir[2] = b[p];

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;

            stackpointer = (stackpointer + 1) % div;
            auto &sir2 = stack[stackpointer];

            routsum += sir2[0];
            goutsum += sir2[1];
            boutsum += sir2[2];

            rinsum -= sir2[0];
            ginsum -= sir2[1];
            binsum -= sir2[2];

            yi += w;
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    jobject result =
            env->CallStaticObjectMethod(
                    bitmapCls,
                    createScaled,
                    bitmap,
                    (jint) (w / scale),
                    (jint) (h / scale),
                    JNI_TRUE
            );

    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_TrickleUtils_trimEmptyPartsImpl(
        JNIEnv *env,
        jobject /* this */,
        jobject bitmap,
        jint transparent
) {
    if (bitmap == nullptr) return nullptr;

    AndroidBitmapInfo info;
    void *pixelsPtr = nullptr;

    if (AndroidBitmap_getInfo(env, bitmap, &info) != ANDROID_BITMAP_RESULT_SUCCESS)
        return bitmap;

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
        return bitmap;

    if (AndroidBitmap_lockPixels(env, bitmap, &pixelsPtr) != ANDROID_BITMAP_RESULT_SUCCESS)
        return bitmap;

    const int width = info.width;
    const int height = info.height;
    uint32_t *pixels = static_cast<uint32_t *>(pixelsPtr);

    int firstX = 0;
    int firstY = 0;
    int lastX = width - 1;
    int lastY = height - 1;

    bool found = false;

    // firstX
    for (int x = 0; x < width && !found; x++) {
        for (int y = 0; y < height; y++) {
            if (pixels[x + y * width] != (uint32_t) transparent) {
                firstX = x;
                found = true;
                break;
            }
        }
    }

    // firstY
    found = false;
    for (int y = 0; y < height && !found; y++) {
        for (int x = firstX; x < width; x++) {
            if (pixels[x + y * width] != (uint32_t) transparent) {
                firstY = y;
                found = true;
                break;
            }
        }
    }

    // lastX
    found = false;
    for (int x = width - 1; x >= firstX && !found; x--) {
        for (int y = height - 1; y >= firstY; y--) {
            if (pixels[x + y * width] != (uint32_t) transparent) {
                lastX = x;
                found = true;
                break;
            }
        }
    }

    // lastY
    found = false;
    for (int y = height - 1; y >= firstY && !found; y--) {
        for (int x = width - 1; x >= firstX; x--) {
            if (pixels[x + y * width] != (uint32_t) transparent) {
                lastY = y;
                found = true;
                break;
            }
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    int outW = lastX - firstX + 1;
    int outH = lastY - firstY + 1;

    if (outW <= 0 || outH <= 0)
        return bitmap;

    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmap =
            env->GetStaticMethodID(
                    bitmapCls,
                    "createBitmap",
                    "(Landroid/graphics/Bitmap;IIII)Landroid/graphics/Bitmap;"
            );

    return env->CallStaticObjectMethod(
            bitmapCls,
            createBitmap,
            bitmap,
            firstX,
            firstY,
            outW,
            outH
    );
}