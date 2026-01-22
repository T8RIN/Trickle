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

    fun fastBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap

    fun stackBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap

    fun glitchVariant(
        src: Bitmap,
        iterations: Int = 30,
        maxOffsetFraction: Float = 0.25f,   // 0f..1f
        channelShiftFraction: Float = 0.3f // 0f..1f
    ): Bitmap

    fun vhsGlitch(
        src: Bitmap,
        time: Float = 2f,
        strength: Float = 3f
    ): Bitmap

    fun blockGlitch(
        src: Bitmap,
        strength: Float = 0.02f,
        blockSizeFraction: Float = 0.5f
    ): Bitmap

    fun crtCurvature(
        src: Bitmap,
        curvature: Float = 0.25f,   // -1f..1f,
        vignette: Float = 0.65f,    // 0f..1f,
        chroma: Float = 0.015f      // 0f..1f
    ): Bitmap

    fun pixelMelt(
        src: Bitmap,
        strength: Float = 0.5f,   // 0f..1f,
        maxDrop: Int = 20
    ): Bitmap

}