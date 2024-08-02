package com.t8rin.trickle.pipeline

import android.graphics.Bitmap
import com.t8rin.trickle.EffectsPipeline

internal object EffectsPipelineImpl : EffectsPipeline {

    override fun oil(
        input: Bitmap,
        oilRange: Int
    ): Bitmap = oilImpl(
        input = input,
        oilRange = oilRange
    ) ?: input

    override fun tv(input: Bitmap): Bitmap = tvImpl(input) ?: input

    override fun hdr(input: Bitmap): Bitmap = hdrImpl(input) ?: input

    override fun sketch(input: Bitmap): Bitmap = sketchImpl(input) ?: input

    override fun gotham(input: Bitmap): Bitmap = gothamImpl(input) ?: input

    override fun cropToContent(
        input: Bitmap,
        colorToIgnore: Int,
        tolerance: Float
    ): Bitmap = cropToContentImpl(
        input = input,
        colorToIgnore = colorToIgnore,
        tolerance = tolerance
    ) ?: input

    override fun transferPalette(
        source: Bitmap,
        target: Bitmap,
        intensity: Float
    ): Bitmap = transferPaletteImpl(
        source = source,
        target = target,
        intensity = intensity
    ) ?: source

    override fun noise(
        input: Bitmap,
        threshold: Int
    ): Bitmap {
        val result = input.copy(input.config, true)
        noiseImpl(
            srcBitmap = result,
            threshold = threshold
        )
        return result
    }

    override fun shuffleBlur(
        input: Bitmap,
        threshold: Float,
        strength: Float
    ): Bitmap {
        val result = input.copy(input.config, true)
        shuffleBlurImpl(
            srcBitmap = result,
            threshold = threshold,
            strength = strength
        )
        return result
    }

    override fun colorPosterize(
        input: Bitmap,
        colors: IntArray
    ): Bitmap = colorPosterizeImpl(
        input = input,
        colors = colors
    ) ?: input

    override fun replaceColor(
        input: Bitmap,
        sourceColor: Int,
        targetColor: Int,
        tolerance: Float
    ): Bitmap = replaceColorImpl(
        input = input,
        sourceColor = sourceColor,
        targetColor = targetColor,
        tolerance = tolerance
    ) ?: input

    override fun drawColorAbove(
        input: Bitmap,
        color: Int
    ): Bitmap = drawColorAboveImpl(
        input = input,
        color = color
    ) ?: input

    override fun drawColorBehind(
        input: Bitmap,
        color: Int
    ): Bitmap = drawColorBehindImpl(
        input = input,
        color = color
    ) ?: input

    override fun tritone(
        input: Bitmap,
        shadowsColor: Int,
        middleColor: Int,
        highlightsColor: Int
    ): Bitmap = tritoneImpl(
        input = input,
        shadowsColor = shadowsColor,
        middleColor = middleColor,
        highlightsColor = highlightsColor
    ) ?: input

    override fun polkaDot(
        input: Bitmap,
        dotRadius: Int,
        spacing: Int
    ): Bitmap = polkaDotImpl(
        input = input,
        dotRadius = dotRadius,
        spacing = spacing
    ) ?: input

    override fun applyLut(
        input: Bitmap,
        lutBitmap: Bitmap,
        intensity: Float
    ): Bitmap = applyLutImpl(
        input = input,
        lutBitmap = lutBitmap,
        intensity = intensity
    )!!

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

    private external fun noiseImpl(srcBitmap: Bitmap, threshold: Int)

    private external fun shuffleBlurImpl(srcBitmap: Bitmap, threshold: Float, strength: Float)

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

}