package com.t8rin.trickle

import android.graphics.Bitmap
import com.t8rin.trickle.pipeline.NativeLoader
import kotlinx.coroutines.runInterruptible

object BmpCompressor : NativeLoader() {

    suspend fun compress(bitmap: Bitmap): ByteArray? = runInterruptible {
        compressNative(bitmap)
    }

    private external fun compressNative(
        bitmap: Bitmap
    ): ByteArray?

}