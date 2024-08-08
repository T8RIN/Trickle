//
// Created by malik on 07.08.2024.
//

#include <jni.h>
#include <android/bitmap.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include "BitmapUtils.h"

struct Patch {
    int x, y;
    float error;
};

// Calculate the error between two patches
float calculatePatchError(uint32_t *srcPixels, int srcWidth, int srcHeight,
                          uint32_t *texPixels, int texWidth, int texHeight,
                          int srcX, int srcY, int texX, int texY, int patchSize) {
    float error = 0.0f;
    for (int j = 0; j < patchSize; ++j) {
        for (int i = 0; i < patchSize; ++i) {
            int srcIndex = (srcY + j) * srcWidth + (srcX + i);
            int texIndex = (texY + j) * texWidth + (texX + i);
            if (srcX + i < srcWidth && srcY + j < srcHeight && texX + i < texWidth &&
                texY + j < texHeight) {
                uint32_t srcColor = srcPixels[srcIndex];
                uint32_t texColor = texPixels[texIndex];
                int rDiff = ((srcColor >> 16) & 0xFF) - ((texColor >> 16) & 0xFF);
                int gDiff = ((srcColor >> 8) & 0xFF) - ((texColor >> 8) & 0xFF);
                int bDiff = (srcColor & 0xFF) - (texColor & 0xFF);
                error += rDiff * rDiff + gDiff * gDiff + bDiff * bDiff;
            }
        }
    }
    return error;
}

// Find the best patch from texture for a given location in the source image
Patch findBestPatch(uint32_t *srcPixels, int srcWidth, int srcHeight,
                    uint32_t *texPixels, int texWidth, int texHeight,
                    int srcX, int srcY, int patchSize) {
    Patch bestPatch = {0, 0, std::numeric_limits<float>::max()};
    for (int texY = 0; texY <= texHeight - patchSize; ++texY) {
        for (int texX = 0; texX <= texWidth - patchSize; ++texX) {
            float error = calculatePatchError(srcPixels, srcWidth, srcHeight,
                                              texPixels, texWidth, texHeight,
                                              srcX, srcY, texX, texY, patchSize);
            if (error < bestPatch.error) {
                bestPatch.error = error;
                bestPatch.x = texX;
                bestPatch.y = texY;
            }
        }
    }
    return bestPatch;
}

// Apply image quilting for texture transfer
void imageQuiltingForTextureTransfer(AndroidBitmapInfo *infoTexture, uint32_t *pixelsTexture,
                                     AndroidBitmapInfo *infoSource, uint32_t *pixelsSource,
                                     uint32_t overlap, uint32_t patchSize, uint32_t *pixelsResult) {
    int width = infoSource->width;
    int height = infoSource->height;

    for (int y = 0; y < height; y += patchSize - overlap) {
        for (int x = 0; x < width; x += patchSize - overlap) {
            Patch bestPatch = findBestPatch(pixelsSource, width, height,
                                            pixelsTexture, infoTexture->width, infoTexture->height,
                                            x, y, patchSize);

            for (int j = 0; j < patchSize; ++j) {
                for (int i = 0; i < patchSize; ++i) {
                    int dstX = x + i;
                    int dstY = y + j;
                    if (dstX < width && dstY < height) {
                        uint32_t texColor = pixelsTexture[(bestPatch.y + j) * infoTexture->width +
                                                          (bestPatch.x + i)];
                        pixelsResult[dstY * width + dstX] = texColor;
                    }
                }
            }
        }
    }
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_t8rin_trickle_pipeline_EffectsPipelineImpl_transferTextureImpl(JNIEnv *env, jobject clazz,
                                                                        jobject textureBitmap,
                                                                        jobject sourceBitmap,
                                                                        jint overlap,
                                                                        jint patchSize) {
    AndroidBitmapInfo infoTexture;
    AndroidBitmapInfo infoSource;
    uint32_t *pixelsTexture;
    uint32_t *pixelsSource;

    if (AndroidBitmap_getInfo(env, textureBitmap, &infoTexture) < 0 ||
        AndroidBitmap_getInfo(env, sourceBitmap, &infoSource) < 0) {
        return nullptr;
    }

    if (infoTexture.format != ANDROID_BITMAP_FORMAT_RGBA_8888 ||
        infoSource.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return nullptr;
    }

    if (AndroidBitmap_lockPixels(env, textureBitmap, (void **) &pixelsTexture) < 0 ||
        AndroidBitmap_lockPixels(env, sourceBitmap, (void **) &pixelsSource) < 0) {
        return nullptr;
    }

    uint32_t *pixelsResult = new uint32_t[infoSource.width * infoSource.height];
    std::fill(pixelsResult, pixelsResult + infoSource.width * infoSource.height,
              0xFFFFFFFF); // Initialize with white background

    imageQuiltingForTextureTransfer(&infoTexture, pixelsTexture,
                                    &infoSource, pixelsSource,
                                    overlap, patchSize, pixelsResult);

    AndroidBitmap_unlockPixels(env, textureBitmap);
    AndroidBitmap_unlockPixels(env, sourceBitmap);

    jobject resultBitmap = createBitmap(env, infoSource.width, infoSource.height);

    uint32_t *pixelsResultBitmap;
    if (AndroidBitmap_lockPixels(env, resultBitmap, (void **) &pixelsResultBitmap) < 0) {
        delete[] pixelsResult;
        return nullptr;
    }

    for (int y = 0; y < infoSource.height; ++y) {
        for (int x = 0; x < infoSource.width; ++x) {
            pixelsResultBitmap[y * infoSource.width + x] = pixelsResult[y * infoSource.width + x];
        }
    }

    AndroidBitmap_unlockPixels(env, resultBitmap);
    delete[] pixelsResult;

    return resultBitmap;
}