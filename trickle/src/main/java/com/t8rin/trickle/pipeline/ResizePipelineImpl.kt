package com.t8rin.trickle.pipeline

import android.graphics.Bitmap
import com.t8rin.trickle.MagicResizeType
import com.t8rin.trickle.ResizePipeline
import com.t8rin.trickle.TrickleUtils.safe

internal object ResizePipelineImpl : ResizePipeline {

    override fun magicResize(
        input: Bitmap,
        width: Int,
        height: Int,
        type: MagicResizeType
    ): Bitmap {
        require(width > 0 && height > 0) {
            "Target width and height must be greater than zero"
        }

        if (input.width == width && input.height == height) return input

        return magicResizeImpl(
            input = input.safe(),
            width = width,
            height = height,
            type = type.ordinal
        ) ?: input
    }

    private external fun magicResizeImpl(
        input: Bitmap,
        width: Int,
        height: Int,
        type: Int
    ): Bitmap?
}
