package com.t8rin.trickle

import android.graphics.Bitmap
import androidx.annotation.IntRange

object Oxipng {

    init {
        System.loadLibrary("oxipng_jni")
    }

    fun optimize(
        bitmap: Bitmap,
        options: Options = Options.DEFAULT,
    ): ByteArray {
        return optimizeBitmapNative(
            bitmap = bitmap.copy(Bitmap.Config.ARGB_8888, false),
            level = options.level,
            stripAll = options.stripAll,
            fixErrors = options.fixErrors,
            force = options.force,
            interlace = options.interlace.value,
            timeoutMs = options.timeoutMs,
        ) ?: error("oxipng: native returned null")
    }

    data class Options(
        @param:IntRange(from = 0, to = 6)
        val level: Int = 2,
        val stripAll: Boolean = false,
        val fixErrors: Boolean = false,
        val force: Boolean = false,
        val interlace: Interlace = Interlace.NONE,
        val timeoutMs: Long = 0L,
    ) {
        companion object {
            val DEFAULT = Options()
            val MAX_COMPRESSION = Options(level = 6, stripAll = true, force = true)
            val FAST = Options(level = 0)
        }
    }

    enum class Interlace(val value: Int) {
        NONE(0),
        ADAM7(1),
        KEEP(-1),
    }

    private external fun optimizeBitmapNative(
        bitmap: Bitmap,
        level: Int,
        stripAll: Boolean,
        fixErrors: Boolean,
        force: Boolean,
        interlace: Int,
        timeoutMs: Long,
    ): ByteArray?
}