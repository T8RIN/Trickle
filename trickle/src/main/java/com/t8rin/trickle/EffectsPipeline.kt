package com.t8rin.trickle

import android.graphics.Bitmap

interface EffectsPipeline {

    suspend fun oil(input: Bitmap, oilRange: Int): Bitmap

    suspend fun tv(input: Bitmap): Bitmap

    suspend fun hdr(input: Bitmap): Bitmap

    suspend fun sketch(input: Bitmap): Bitmap

    suspend fun gotham(input: Bitmap): Bitmap

    suspend fun cropToContent(
        input: Bitmap,
        colorToIgnore: Int,
        tolerance: Float
    ): Bitmap

    suspend fun transferPalette(
        source: Bitmap,
        target: Bitmap,
        intensity: Float
    ): Bitmap

    suspend fun noise(
        input: Bitmap,
        threshold: Int
    ): Bitmap

    suspend fun shuffleBlur(
        input: Bitmap,
        threshold: Float,
        strength: Float
    ): Bitmap

    suspend fun colorPosterize(
        input: Bitmap,
        colors: IntArray
    ): Bitmap

    suspend fun replaceColor(
        input: Bitmap,
        sourceColor: Int,
        targetColor: Int,
        tolerance: Float
    ): Bitmap

    suspend fun drawColorAbove(
        input: Bitmap,
        color: Int
    ): Bitmap

    suspend fun drawColorBehind(
        input: Bitmap,
        color: Int
    ): Bitmap

    suspend fun tritone(
        input: Bitmap,
        shadowsColor: Int,
        middleColor: Int,
        highlightsColor: Int
    ): Bitmap

    suspend fun polkaDot(
        input: Bitmap,
        dotRadius: Int,
        spacing: Int
    ): Bitmap

    suspend fun applyLut(
        input: Bitmap,
        lutBitmap: Bitmap,
        intensity: Float
    ): Bitmap

    suspend fun applyCubeLut(
        input: Bitmap,
        cubeLutPath: String,
        intensity: Float
    ): Bitmap

    suspend fun popArt(
        input: Bitmap,
        color: Int,
        blendMode: PopArtBlendMode,
        strength: Float
    ): Bitmap

    suspend fun fastBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap

    suspend fun stackBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap

    suspend fun glitchVariant(
        src: Bitmap,
        iterations: Int = 30,
        maxOffsetFraction: Float = 0.25f,   // 0f..1f
        channelShiftFraction: Float = 0.3f // 0f..1f
    ): Bitmap

    suspend fun vhsGlitch(
        src: Bitmap,
        time: Float = 2f,
        strength: Float = 3f
    ): Bitmap

    suspend fun ntsc(
        src: Bitmap,
        settings: NtscSettings = NtscSettings.DEFAULT,
        frame: Int = 0,
        scaleFactorX: Float = 1f,
        scaleFactorY: Float = 1f
    ): Bitmap

    suspend fun blockGlitch(
        src: Bitmap,
        strength: Float = 0.02f,
        blockSizeFraction: Float = 0.5f
    ): Bitmap

    suspend fun crtCurvature(
        src: Bitmap,
        curvature: Float = 0.25f,   // -1f..1f,
        vignette: Float = 0.65f,    // 0f..1f,
        chroma: Float = 0.015f      // 0f..1f
    ): Bitmap

    suspend fun pixelMelt(
        src: Bitmap,
        strength: Float = 0.5f,   // 0f..1f,
        maxDrop: Int = 20
    ): Bitmap

    suspend fun bloom(
        src: Bitmap,
        threshold: Float, // 0f..1f,
        intensity: Float,
        radius: Int,
        softKnee: Float,
        exposure: Float,
        gamma: Float
    ): Bitmap

    suspend fun autoWhiteBalance(
        input: Bitmap,
        strength: Float = 1f,
        clipPercent: Float = 0.05f
    ): Bitmap

}
