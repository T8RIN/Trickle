#include <jni.h>
#include <android/bitmap.h>
#include <vector>
#include <cstring>

#define BYTE_PER_PIXEL 3
#define BMP_WIDTH_OF_TIMES 4

static inline void writeInt(std::vector<uint8_t> &buf, int value) {
    buf.push_back(value & 0xFF);
    buf.push_back((value >> 8) & 0xFF);
    buf.push_back((value >> 16) & 0xFF);
    buf.push_back((value >> 24) & 0xFF);
}

static inline void writeShort(std::vector<uint8_t> &buf, short value) {
    buf.push_back(value & 0xFF);
    buf.push_back((value >> 8) & 0xFF);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_t8rin_trickle_BmpCompressor_compress(
        JNIEnv *env,
        jobject /* this */,
        jobject bitmap
) {
    AndroidBitmapInfo info;
    void *pixelsPtr;
    int lock;

    lock = AndroidBitmap_getInfo(env, bitmap, &info);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    lock = AndroidBitmap_lockPixels(env, bitmap, &pixelsPtr);
    if (lock != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return nullptr;
    }

    int width = info.width;
    int height = info.height;

    int rowWidthInBytes = BYTE_PER_PIXEL * width;
    bool hasDummy = false;
    std::vector<uint8_t> dummyBytesPerRow;

    if (rowWidthInBytes % BMP_WIDTH_OF_TIMES > 0) {
        hasDummy = true;
        int dummySize = BMP_WIDTH_OF_TIMES - (rowWidthInBytes % BMP_WIDTH_OF_TIMES);
        dummyBytesPerRow.resize(dummySize, 0xFF);
    }

    int imageSize = (rowWidthInBytes + (hasDummy ? dummyBytesPerRow.size() : 0)) * height;
    int imageDataOffset = 0x36;
    int fileSize = imageSize + imageDataOffset;

    std::vector<uint8_t> buffer;
    buffer.reserve(fileSize);

    // BITMAP FILE HEADER
    buffer.push_back(0x42);
    buffer.push_back(0x4D);
    writeInt(buffer, fileSize);
    writeShort(buffer, 0);
    writeShort(buffer, 0);
    writeInt(buffer, imageDataOffset);

    // BITMAP INFO HEADER
    writeInt(buffer, 0x28);
    writeInt(buffer, width + (hasDummy && dummyBytesPerRow.size() == 3 ? 1 : 0));
    writeInt(buffer, height);
    writeShort(buffer, 1);
    writeShort(buffer, 24);
    writeInt(buffer, 0);
    writeInt(buffer, imageSize);
    writeInt(buffer, 0);
    writeInt(buffer, 0);
    writeInt(buffer, 0);
    writeInt(buffer, 0);

    uint32_t *src = static_cast<uint32_t *>(pixelsPtr);

    for (int row = height - 1; row >= 0; row--) {
        int offset = row * width;
        for (int col = 0; col < width; col++) {
            uint32_t px = src[offset + col];

            uint8_t r = (px >> 16) & 0xFF;
            uint8_t g = (px >> 8) & 0xFF;
            uint8_t b = px & 0xFF;

            buffer.push_back(r);
            buffer.push_back(g);
            buffer.push_back(b);
        }
        if (hasDummy) {
            buffer.insert(buffer.end(), dummyBytesPerRow.begin(), dummyBytesPerRow.end());
        }
    }

    AndroidBitmap_unlockPixels(env, bitmap);

    jbyteArray result = env->NewByteArray(buffer.size());
    env->SetByteArrayRegion(result, 0, buffer.size(),
            reinterpret_cast<jbyte *>(buffer.data()));
    return result;
}