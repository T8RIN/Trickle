package com.t8rin.trickle

import android.graphics.Bitmap

interface EffectsPipeline {

    fun oil(input: Bitmap, oilRange: Int): Bitmap

    fun tv(input: Bitmap): Bitmap

    fun hdr(input: Bitmap): Bitmap

    fun sketch(input: Bitmap): Bitmap

    fun gotham(input: Bitmap): Bitmap

    fun cropToContent(
        input: Bitmap,
        colorToIgnore: Int,
        tolerance: Float
    ): Bitmap

    fun transferPalette(
        source: Bitmap,
        target: Bitmap,
        intensity: Float
    ): Bitmap

    fun noise(
        input: Bitmap,
        threshold: Int
    ): Bitmap

    fun shuffleBlur(
        input: Bitmap,
        threshold: Float,
        strength: Float
    ): Bitmap

    fun colorPosterize(
        input: Bitmap,
        colors: IntArray
    ): Bitmap

    fun replaceColor(
        input: Bitmap,
        sourceColor: Int,
        targetColor: Int,
        tolerance: Float
    ): Bitmap

    fun drawColorAbove(
        input: Bitmap,
        color: Int
    ): Bitmap

    fun drawColorBehind(
        input: Bitmap,
        color: Int
    ): Bitmap

    fun tritone(
        input: Bitmap,
        shadowsColor: Int,
        middleColor: Int,
        highlightsColor: Int
    ): Bitmap

    fun polkaDot(
        input: Bitmap,
        dotRadius: Int,
        spacing: Int
    ): Bitmap

    fun applyLut(
        input: Bitmap,
        lutBitmap: Bitmap,
        intensity: Float
    ): Bitmap

    fun applyCubeLut(
        input: Bitmap,
        cubeLutPath: String,
        intensity: Float
    ): Bitmap

    fun popArt(
        input: Bitmap,
        color: Int,
        blendMode: PopArtBlendMode,
        strength: Float
    ): Bitmap

}