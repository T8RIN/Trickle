@file:Suppress("unused", "FunctionName")

package com.t8rin.trickle

import android.graphics.Bitmap
import androidx.annotation.IntRange
import com.t8rin.trickle.TrickleUtils.safe

object Oxipng {

    init {
        System.loadLibrary("oxipng_jni")
    }

    fun optimize(
        bitmap: Bitmap,
        options: Options = Options.DEFAULT,
    ): ByteArray {
        return optimizeBitmapNative(
            bitmap = bitmap.safe(),
            level = options.level,
            stripAll = options.stripAll,
            fixErrors = options.fixErrors,
            force = options.force,
            interlace = options.interlace.value,
            timeoutMs = options.timeoutMs,
            useZopfli = options.useZopfli
        ) ?: error("oxipng: native returned null")
    }

    data class Options(
        @param:IntRange(from = 0, to = 6)
        val level: Int = 2,
        val stripAll: Boolean = true,
        val fixErrors: Boolean = false,
        val force: Boolean = false,
        val interlace: Interlace = Interlace.NONE,
        val timeoutMs: Long = 0L,
        val useZopfli: Boolean = false,
    ) {
        companion object {
            val DEFAULT = Options()
            val MAX_COMPRESSION = Options(
                level = 6,
                stripAll = true,
                force = true,
                useZopfli = true
            )
            val FAST = Options(level = 0)
        }
    }

    fun SimpleOptions(
        @IntRange(from = 0, to = 6)
        level: Int,
        stripAll: Boolean = true,
        useZopfli: Boolean = false
    ) = Options(
        level = level,
        stripAll = stripAll,
        useZopfli = useZopfli
    )

    enum class Interlace(val value: Int) {
        NONE(0),
        ADAM7(1),
        KEEP(-1),
    }

    @Suppress("KotlinJniMissingFunction")
    private external fun optimizeBitmapNative(
        bitmap: Bitmap,
        level: Int,
        stripAll: Boolean,
        fixErrors: Boolean,
        force: Boolean,
        interlace: Int,
        timeoutMs: Long,
        useZopfli: Boolean,
    ): ByteArray?
}