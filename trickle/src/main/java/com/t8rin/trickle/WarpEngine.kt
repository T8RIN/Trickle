package com.t8rin.trickle

import android.graphics.Bitmap

class WarpEngine(
    src: Bitmap
) {
    private val handle: Long

    init {
        handle = nativeCreate(src)
    }

    fun applyStroke(
        fromX: Float,
        fromY: Float,
        toX: Float,
        toY: Float,
        brush: WarpBrush,
        mode: WarpMode
    ) {
        nativeApplyStroke(
            handle = handle,
            fromX = fromX,
            fromY = fromY,
            toX = toX,
            toY = toY,
            radius = brush.radius,
            hardness = brush.hardness,
            strength = brush.strength,
            mode = mode.ordinal
        )
    }

    fun render(): Bitmap = nativeRender(handle)

    fun release() = nativeDestroy(handle)

    private external fun nativeCreate(src: Bitmap): Long
    private external fun nativeApplyStroke(
        handle: Long,
        fromX: Float,
        fromY: Float,
        toX: Float,
        toY: Float,
        radius: Float,
        hardness: Float,
        strength: Float,
        mode: Int
    )

    private external fun nativeRender(handle: Long): Bitmap
    private external fun nativeDestroy(handle: Long)

    companion object {
        init {
            System.loadLibrary("trickle")
        }
    }
}

enum class WarpMode {
    MOVE,
    GROW,
    SHRINK,
    SWIRL_CW,
    SWIRL_CCW
}

data class WarpBrush(
    val radius: Float,
    val strength: Float,
    val hardness: Float
)