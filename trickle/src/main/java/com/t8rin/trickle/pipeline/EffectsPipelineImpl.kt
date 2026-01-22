package com.t8rin.trickle.pipeline

import android.graphics.Bitmap
import android.graphics.Color
import com.t8rin.trickle.EffectsPipeline
import com.t8rin.trickle.PopArtBlendMode
import com.t8rin.trickle.TrickleUtils.safe

internal object EffectsPipelineImpl : EffectsPipeline {

    override fun oil(
        input: Bitmap,
        oilRange: Int
    ): Bitmap = oilImpl(
        input = input.safe(),
        oilRange = oilRange
    ) ?: input

    override fun tv(
        input: Bitmap
    ): Bitmap = tvImpl(input.safe()) ?: input

    override fun hdr(
        input: Bitmap
    ): Bitmap = hdrImpl(input.safe()) ?: input

    override fun sketch(
        input: Bitmap
    ): Bitmap = sketchImpl(input.safe()) ?: input

    override fun gotham(
        input: Bitmap
    ): Bitmap = gothamImpl(input.safe()) ?: input

    override fun cropToContent(
        input: Bitmap,
        colorToIgnore: Int,
        tolerance: Float
    ): Bitmap = cropToContentImpl(
        input = input.safe(),
        colorToIgnore = colorToIgnore,
        tolerance = tolerance
    ) ?: input

    override fun transferPalette(
        source: Bitmap,
        target: Bitmap,
        intensity: Float
    ): Bitmap = transferPaletteImpl(
        source = source.safe(),
        target = target.safe(),
        intensity = intensity
    ) ?: source

    override fun noise(
        input: Bitmap,
        threshold: Int
    ): Bitmap = noiseImpl(
        srcBitmap = input.safe(),
        threshold = threshold
    ) ?: input

    override fun shuffleBlur(
        input: Bitmap,
        threshold: Float,
        strength: Float
    ): Bitmap = shuffleBlurImpl(
        srcBitmap = input.safe(),
        threshold = threshold,
        strength = strength
    ) ?: input

    override fun colorPosterize(
        input: Bitmap,
        colors: IntArray
    ): Bitmap = colorPosterizeImpl(
        input = input.safe(),
        colors = colors
    ) ?: input

    override fun replaceColor(
        input: Bitmap,
        sourceColor: Int,
        targetColor: Int,
        tolerance: Float
    ): Bitmap = replaceColorImpl(
        input = input.safe(),
        sourceColor = sourceColor,
        targetColor = targetColor,
        tolerance = tolerance
    ) ?: input

    override fun drawColorAbove(
        input: Bitmap,
        color: Int
    ): Bitmap = drawColorAboveImpl(
        input = input.safe(),
        color = color
    ) ?: input

    override fun drawColorBehind(
        input: Bitmap,
        color: Int
    ): Bitmap = drawColorBehindImpl(
        input = input.safe(),
        color = color
    ) ?: input

    override fun tritone(
        input: Bitmap,
        shadowsColor: Int,
        middleColor: Int,
        highlightsColor: Int
    ): Bitmap = tritoneImpl(
        input = input.safe(),
        shadowsColor = shadowsColor,
        middleColor = middleColor,
        highlightsColor = highlightsColor
    ) ?: input

    override fun polkaDot(
        input: Bitmap,
        dotRadius: Int,
        spacing: Int
    ): Bitmap = polkaDotImpl(
        input = input.safe(),
        dotRadius = dotRadius,
        spacing = spacing
    ) ?: input

    override fun applyLut(
        input: Bitmap,
        lutBitmap: Bitmap,
        intensity: Float
    ): Bitmap = applyLutImpl(
        input = input.safe(),
        lutBitmap = lutBitmap.safe(),
        intensity = intensity
    ) ?: input

    override fun applyCubeLut(
        input: Bitmap,
        cubeLutPath: String,
        intensity: Float
    ): Bitmap = applyCubeLutImpl(
        input = input.safe(),
        cubeLutPath = cubeLutPath,
        intensity = intensity
    ) ?: input

    override fun popArt(
        input: Bitmap,
        color: Int,
        blendMode: PopArtBlendMode,
        strength: Float
    ): Bitmap {
        val (firstColor, secondColor, thirdColor, fourthColor) = squareHarmony(color)
        return popArtImpl(
            input = input.safe(),
            firstColor = firstColor,
            secondColor = secondColor,
            thirdColor = thirdColor,
            fourthColor = fourthColor,
            blendMode = blendMode.ordinal,
            strength = strength
        ) ?: input
    }

    override fun stackBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap = stackBlurImpl(
        srcBitmap = bitmap.safe(),
        scale = scale,
        radius = radius
    )

    override fun fastBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap = fastBlurImpl(
        srcBitmap = bitmap.safe(),
        scale = scale,
        radius = radius
    )

    override fun glitchVariant(
        src: Bitmap,
        iterations: Int,
        maxOffsetFraction: Float,
        channelShiftFraction: Float
    ): Bitmap = glitchVariantImpl(
        src = src.safe(),
        iterations = iterations,
        maxOffsetFraction = maxOffsetFraction,
        channelShiftFraction = channelShiftFraction
    ) ?: src

    private external fun glitchVariantImpl(
        src: Bitmap,
        iterations: Int,
        maxOffsetFraction: Float,
        channelShiftFraction: Float
    ): Bitmap?

    override fun vhsGlitch(
        src: Bitmap,
        time: Float,
        strength: Float
    ): Bitmap = vhsGlitchImpl(
        src = src.safe(),
        time = time,
        strength = strength
    ) ?: src

    private external fun vhsGlitchImpl(
        src: Bitmap,
        time: Float,
        strength: Float
    ): Bitmap?

    override fun blockGlitch(
        src: Bitmap,
        strength: Float,
        blockSizeFraction: Float
    ): Bitmap = blockGlitchImpl(
        src = src.safe(),
        strength = strength,
        blockSizeFraction = blockSizeFraction
    ) ?: src

    private external fun blockGlitchImpl(
        src: Bitmap,
        strength: Float,
        blockSizeFraction: Float
    ): Bitmap?

    override fun crtCurvature(
        src: Bitmap,
        curvature: Float,
        vignette: Float,
        chroma: Float
    ): Bitmap = crtCurvatureImpl(
        src = src.safe(),
        curvature = curvature,
        vignette = vignette,
        chroma = chroma
    ) ?: src

    private external fun crtCurvatureImpl(
        src: Bitmap,
        curvature: Float,
        vignette: Float,
        chroma: Float
    ): Bitmap?

    override fun pixelMelt(
        src: Bitmap,
        strength: Float,
        maxDrop: Int
    ): Bitmap = pixelMeltImpl(
        src = src.safe(),
        strength = strength,
        maxDrop = maxDrop
    ) ?: src

    private external fun pixelMeltImpl(
        src: Bitmap,
        strength: Float,
        maxDrop: Int
    ): Bitmap?

    private external fun popArtImpl(
        input: Bitmap,
        firstColor: Int,
        secondColor: Int,
        thirdColor: Int,
        fourthColor: Int,
        blendMode: Int,
        strength: Float
    ): Bitmap?

    private external fun applyCubeLutImpl(
        input: Bitmap,
        cubeLutPath: String,
        intensity: Float
    ): Bitmap?

    private external fun applyLutImpl(
        input: Bitmap,
        lutBitmap: Bitmap,
        intensity: Float
    ): Bitmap?

    private external fun tritoneImpl(
        input: Bitmap,
        shadowsColor: Int,
        middleColor: Int,
        highlightsColor: Int
    ): Bitmap?

    private external fun oilImpl(input: Bitmap, oilRange: Int): Bitmap?

    private external fun tvImpl(input: Bitmap): Bitmap?

    private external fun hdrImpl(input: Bitmap): Bitmap?

    private external fun sketchImpl(input: Bitmap): Bitmap?

    private external fun gothamImpl(input: Bitmap): Bitmap?

    private external fun cropToContentImpl(
        input: Bitmap,
        colorToIgnore: Int,
        tolerance: Float
    ): Bitmap?

    private external fun transferPaletteImpl(
        source: Bitmap,
        target: Bitmap,
        intensity: Float
    ): Bitmap?

    private external fun noiseImpl(
        srcBitmap: Bitmap,
        threshold: Int
    ): Bitmap?

    private external fun shuffleBlurImpl(
        srcBitmap: Bitmap,
        threshold: Float,
        strength: Float
    ): Bitmap?

    private external fun colorPosterizeImpl(
        input: Bitmap,
        colors: IntArray
    ): Bitmap?

    private external fun replaceColorImpl(
        input: Bitmap,
        sourceColor: Int,
        targetColor: Int,
        tolerance: Float
    ): Bitmap?

    private external fun drawColorAboveImpl(
        input: Bitmap,
        color: Int
    ): Bitmap?

    private external fun drawColorBehindImpl(
        input: Bitmap,
        color: Int
    ): Bitmap?

    private external fun polkaDotImpl(
        input: Bitmap,
        dotRadius: Int,
        spacing: Int
    ): Bitmap?

    private external fun stackBlurImpl(
        srcBitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap

    private external fun fastBlurImpl(
        srcBitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap

}

private fun squareHarmony(color: Int): List<Int> {
    val (h, s, v) = color.toHSV()
    return listOf(
        hsvToColor(h, s, v),
        hsvToColor((h + 90) % 360, s, v),
        hsvToColor((h + 180) % 360, s, v),
        hsvToColor((h + 270) % 360, s, v)
    )
}

private fun Int.toHSV(): FloatArray {
    val hsv = FloatArray(3)
    Color.colorToHSV(this, hsv)
    return hsv
}

private fun hsvToColor(
    h: Float,
    s: Float,
    v: Float
): Int {
    return Color.HSVToColor(floatArrayOf(h, s, v))
}