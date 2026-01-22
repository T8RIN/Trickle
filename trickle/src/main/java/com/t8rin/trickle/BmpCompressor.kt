package com.t8rin.trickle

import android.graphics.Bitmap
import com.t8rin.trickle.pipeline.NativeLoader

object BmpCompressor : NativeLoader() {

    external fun compress(
        bitmap: Bitmap
    ): ByteArray?

}