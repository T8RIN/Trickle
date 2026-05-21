@file:Suppress("unused", "FunctionName")

package com.t8rin.trickle

import android.graphics.Bitmap
import androidx.annotation.FloatRange
import androidx.annotation.IntRange
import com.t8rin.trickle.TrickleUtils.safe

object ImageQuant {

    init {
        System.loadLibrary("imagequant_jni")
    }

    fun compress(
        bitmap: Bitmap,
        options: Options = Options(),
    ): ByteArray {
        return compressBitmapNative(
            bitmap = bitmap.safe(),
            maxColors = options.maxColors,
            minQuality = options.minQuality,
            targetQuality = options.targetQuality,
            speed = options.speed,
            minPosterization = options.minPosterization,
            ditheringLevel = options.ditheringLevel,
            inputGamma = options.inputGamma,
            outputGamma = options.outputGamma,
            lastIndexTransparent = options.lastIndexTransparent,
            bitDepth = options.bitDepth.value,
            pngCompression = options.pngCompression.value,
            pngFilter = options.pngFilter.value,
            adaptiveFilter = options.adaptiveFilter
        ) ?: error("imagequant: native returned null")
    }

    data class Options(
        @param:IntRange(from = 2)
        val maxColors: Int = 256,
        @param:IntRange(from = 0, to = 100)
        val minQuality: Int = 0,
        @param:IntRange(from = 0, to = 100)
        val targetQuality: Int = 100,
        @param:IntRange(from = 1, to = 10)
        val speed: Int = 4,
        @param:IntRange(from = 0, to = 4)
        val minPosterization: Int = 0,
        @param:FloatRange(from = 0.0, to = 1.0)
        val ditheringLevel: Float = 1f,
        val inputGamma: Double = 0.0,
        val outputGamma: Double = 0.0,
        val lastIndexTransparent: Boolean = false,
        val bitDepth: PngBitDepth = PngBitDepth.AUTO,
        val pngCompression: PngCompression = PngCompression.BEST,
        val pngFilter: PngFilter = PngFilter.DEFAULT,
        val adaptiveFilter: Boolean = true,
    ) {
        constructor(
            @IntRange(from = 0, to = 100)
            quality: Int,
            @IntRange(from = 1, to = 10)
            speed: Int,
            @IntRange(from = 2)
            maxColors: Int = 1024,
            @FloatRange(from = 0.0, to = 1.0)
            ditheringLevel: Float = 1f
        ) : this(
            maxColors = maxColors,
            targetQuality = quality,
            speed = speed,
            ditheringLevel = ditheringLevel
        )
    }

    enum class PngBitDepth(val value: Int) {
        AUTO(0),
        ONE(1),
        TWO(2),
        FOUR(4),
        EIGHT(8),
    }

    enum class PngCompression(val value: Int) {
        DEFAULT(0),
        FAST(1),
        BEST(2),
    }

    enum class PngFilter(val value: Int) {
        DEFAULT(-1),
        NONE(0),
        SUB(1),
        UP(2),
        AVG(3),
        PAETH(4),
        ADAPTIVE(5),
    }

    @Suppress("KotlinJniMissingFunction")
    private external fun compressBitmapNative(
        bitmap: Bitmap,
        maxColors: Int,
        minQuality: Int,
        targetQuality: Int,
        speed: Int,
        minPosterization: Int,
        ditheringLevel: Float,
        inputGamma: Double,
        outputGamma: Double,
        lastIndexTransparent: Boolean,
        bitDepth: Int,
        pngCompression: Int,
        pngFilter: Int,
        adaptiveFilter: Boolean,
    ): ByteArray?
}
