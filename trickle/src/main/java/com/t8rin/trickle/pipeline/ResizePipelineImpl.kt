package com.t8rin.trickle.pipeline

import android.graphics.Bitmap
import com.t8rin.trickle.MagicResizeType
import com.t8rin.trickle.ResizePipeline
import com.t8rin.trickle.TrickleUtils.safe
import kotlinx.coroutines.runInterruptible

internal object ResizePipelineImpl : ResizePipeline {

    override suspend fun magicResize(
        input: Bitmap,
        width: Int,
        height: Int,
        type: MagicResizeType
    ): Bitmap = runInterruptible {
        require(width > 0 && height > 0) {
            "Target width and height must be greater than zero"
        }

        if (input.width == width && input.height == height) return@runInterruptible input

        magicResizeImpl(
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
