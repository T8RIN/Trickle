package com.t8rin.trickle

import android.graphics.Bitmap

interface ResizePipeline {

    fun magicResize(
        input: Bitmap,
        width: Int,
        height: Int,
        type: MagicResizeType
    ): Bitmap
}
