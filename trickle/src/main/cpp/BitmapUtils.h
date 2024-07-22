//
// Created by malik on 22.07.2024.
//

#ifndef TRICKLELIB_BITMAPUTILS_H
#define TRICKLELIB_BITMAPUTILS_H

#include <jni.h>

jobject createBitmap(JNIEnv *env, int *targetPixels, int width, int height, int stride,
                     bool autoDelete = false);

jobject createBitmap(JNIEnv *env, int width, int height);

#endif //TRICKLELIB_BITMAPUTILS_H
