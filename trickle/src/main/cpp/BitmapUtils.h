//
// Created by malik on 22.07.2024.
//

#ifndef TRICKLELIB_BITMAPUTILS_H
#define TRICKLELIB_BITMAPUTILS_H

#include <jni.h>

jobject createBitmap(JNIEnv *env, int *targetPixels, int width, int height, int stride,
                     bool autoDelete = false);

jobject createBitmap(JNIEnv *env, uint32_t width, uint32_t height);

#endif //TRICKLELIB_BITMAPUTILS_H
