package com.t8rin.trickle.pipeline

import android.graphics.Bitmap
import android.graphics.Color
import com.t8rin.trickle.EffectsPipeline
import com.t8rin.trickle.NtscSettings
import com.t8rin.trickle.PopArtBlendMode
import com.t8rin.trickle.TrickleUtils.safe
import kotlinx.coroutines.runInterruptible

internal object EffectsPipelineImpl : EffectsPipeline {

    private val ntscLibrary = lazy {
        System.loadLibrary("ntsc_jni")
    }

    override suspend fun oil(
        input: Bitmap,
        oilRange: Int
    ): Bitmap = runInterruptible {
        oilImpl(
            input = input.safe(),
            oilRange = oilRange
        ) ?: input
    }

    override suspend fun tv(
        input: Bitmap
    ): Bitmap = runInterruptible {
        tvImpl(input.safe()) ?: input
    }

    override suspend fun hdr(
        input: Bitmap
    ): Bitmap = runInterruptible {
        hdrImpl(input.safe()) ?: input
    }

    override suspend fun sketch(
        input: Bitmap
    ): Bitmap = runInterruptible {
        sketchImpl(input.safe()) ?: input
    }

    override suspend fun gotham(
        input: Bitmap
    ): Bitmap = runInterruptible {
        gothamImpl(input.safe()) ?: input
    }

    override suspend fun cropToContent(
        input: Bitmap,
        colorToIgnore: Int,
        tolerance: Float
    ): Bitmap = runInterruptible {
        cropToContentImpl(
            input = input.safe(),
            colorToIgnore = colorToIgnore,
            tolerance = tolerance
        ) ?: input
    }

    override suspend fun transferPalette(
        source: Bitmap,
        target: Bitmap,
        intensity: Float
    ): Bitmap = runInterruptible {
        transferPaletteImpl(
            source = source.safe(),
            target = target.safe(),
            intensity = intensity
        ) ?: source
    }

    override suspend fun noise(
        input: Bitmap,
        threshold: Int
    ): Bitmap = runInterruptible {
        noiseImpl(
            srcBitmap = input.safe(),
            threshold = threshold
        ) ?: input
    }

    override suspend fun shuffleBlur(
        input: Bitmap,
        threshold: Float,
        strength: Float
    ): Bitmap = runInterruptible {
        shuffleBlurImpl(
            srcBitmap = input.safe(),
            threshold = threshold,
            strength = strength
        ) ?: input
    }

    override suspend fun colorPosterize(
        input: Bitmap,
        colors: IntArray
    ): Bitmap = runInterruptible {
        colorPosterizeImpl(
            input = input.safe(),
            colors = colors
        ) ?: input
    }

    override suspend fun replaceColor(
        input: Bitmap,
        sourceColor: Int,
        targetColor: Int,
        tolerance: Float
    ): Bitmap = runInterruptible {
        replaceColorImpl(
            input = input.safe(),
            sourceColor = sourceColor,
            targetColor = targetColor,
            tolerance = tolerance
        ) ?: input
    }

    override suspend fun drawColorAbove(
        input: Bitmap,
        color: Int
    ): Bitmap = runInterruptible {
        drawColorAboveImpl(
            input = input.safe(),
            color = color
        ) ?: input
    }

    override suspend fun drawColorBehind(
        input: Bitmap,
        color: Int
    ): Bitmap = runInterruptible {
        drawColorBehindImpl(
            input = input.safe(),
            color = color
        ) ?: input
    }

    override suspend fun tritone(
        input: Bitmap,
        shadowsColor: Int,
        middleColor: Int,
        highlightsColor: Int
    ): Bitmap = runInterruptible {
        tritoneImpl(
            input = input.safe(),
            shadowsColor = shadowsColor,
            middleColor = middleColor,
            highlightsColor = highlightsColor
        ) ?: input
    }

    override suspend fun polkaDot(
        input: Bitmap,
        dotRadius: Int,
        spacing: Int
    ): Bitmap = runInterruptible {
        polkaDotImpl(
            input = input.safe(),
            dotRadius = dotRadius,
            spacing = spacing
        ) ?: input
    }

    override suspend fun applyLut(
        input: Bitmap,
        lutBitmap: Bitmap,
        intensity: Float
    ): Bitmap = runInterruptible {
        applyLutImpl(
            input = input.safe(),
            lutBitmap = lutBitmap.safe(),
            intensity = intensity
        ) ?: input
    }

    override suspend fun applyCubeLut(
        input: Bitmap,
        cubeLutPath: String,
        intensity: Float
    ): Bitmap = runInterruptible {
        applyCubeLutImpl(
            input = input.safe(),
            cubeLutPath = cubeLutPath,
            intensity = intensity
        ) ?: input
    }

    override suspend fun popArt(
        input: Bitmap,
        color: Int,
        blendMode: PopArtBlendMode,
        strength: Float
    ): Bitmap = runInterruptible {
        val (firstColor, secondColor, thirdColor, fourthColor) = squareHarmony(color)
        popArtImpl(
            input = input.safe(),
            firstColor = firstColor,
            secondColor = secondColor,
            thirdColor = thirdColor,
            fourthColor = fourthColor,
            blendMode = blendMode.ordinal,
            strength = strength
        ) ?: input
    }

    override suspend fun stackBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap = runInterruptible {
        stackBlurImpl(
            srcBitmap = bitmap.safe(),
            scale = scale,
            radius = radius
        )
    }

    override suspend fun fastBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap = runInterruptible {
        fastBlurImpl(
            srcBitmap = bitmap.safe(),
            scale = scale,
            radius = radius
        )
    }

    override suspend fun glitchVariant(
        src: Bitmap,
        iterations: Int,
        maxOffsetFraction: Float,
        channelShiftFraction: Float
    ): Bitmap = runInterruptible {
        glitchVariantImpl(
            src = src.safe(),
            iterations = iterations,
            maxOffsetFraction = maxOffsetFraction,
            channelShiftFraction = channelShiftFraction
        ) ?: src
    }

    private external fun glitchVariantImpl(
        src: Bitmap,
        iterations: Int,
        maxOffsetFraction: Float,
        channelShiftFraction: Float
    ): Bitmap?

    override suspend fun vhsGlitch(
        src: Bitmap,
        time: Float,
        strength: Float
    ): Bitmap = runInterruptible {
        vhsGlitchImpl(
            src = src.safe(),
            time = time,
            strength = strength
        ) ?: src
    }

    private external fun vhsGlitchImpl(
        src: Bitmap,
        time: Float,
        strength: Float
    ): Bitmap?

    override suspend fun ntsc(
        src: Bitmap,
        settings: NtscSettings,
        frame: Int,
        scaleFactorX: Float,
        scaleFactorY: Float
    ): Bitmap = runInterruptible {
        ntscLibrary.value

        val headSwitching = settings.headSwitching
        val headSwitchingMidLine = headSwitching?.midLine
        val trackingNoise = settings.trackingNoise
        val compositeNoise = settings.compositeNoise
        val ringing = settings.ringing
        val lumaNoise = settings.lumaNoise
        val chromaNoise = settings.chromaNoise
        val vhs = settings.vhs
        val vhsSharpen = vhs?.sharpen
        val vhsEdgeWave = vhs?.edgeWave
        val scale = settings.scale

        ntscImpl(
            input = src.safe(),
            frame = frame,
            scaleFactorX = scaleFactorX,
            scaleFactorY = scaleFactorY,
            intSettings = intArrayOf(
                settings.randomSeed,
                settings.useField.ordinal,
                settings.filterType.ordinal,
                settings.inputLumaFilter.ordinal,
                settings.chromaLowpassIn.ordinal,
                settings.chromaDemodulation.ordinal,
                settings.videoScanlinePhaseShift.ordinal,
                settings.videoScanlinePhaseShiftOffset,
                headSwitching?.height ?: 0,
                headSwitching?.offset ?: 0,
                trackingNoise?.height ?: 0,
                compositeNoise?.detail ?: 0,
                lumaNoise?.detail ?: 0,
                chromaNoise?.detail ?: 0,
                settings.chromaDelayVertical,
                vhs?.tapeSpeed?.ordinal ?: 0,
                vhsEdgeWave?.detail ?: 0,
                settings.chromaLowpassOut.ordinal
            ),
            floatSettings = floatArrayOf(
                settings.lumaSmear,
                settings.compositeSharpening,
                headSwitching?.horizontalShift ?: 0f,
                headSwitchingMidLine?.position ?: 0f,
                headSwitchingMidLine?.jitter ?: 0f,
                trackingNoise?.waveIntensity ?: 0f,
                trackingNoise?.snowIntensity ?: 0f,
                trackingNoise?.snowAnisotropy ?: 0f,
                trackingNoise?.noiseIntensity ?: 0f,
                compositeNoise?.frequency ?: 0f,
                compositeNoise?.intensity ?: 0f,
                ringing?.frequency ?: 0f,
                ringing?.power ?: 0f,
                ringing?.intensity ?: 0f,
                lumaNoise?.frequency ?: 0f,
                lumaNoise?.intensity ?: 0f,
                chromaNoise?.frequency ?: 0f,
                chromaNoise?.intensity ?: 0f,
                settings.snowIntensity,
                settings.snowAnisotropy,
                settings.chromaPhaseNoiseIntensity,
                settings.chromaPhaseError,
                settings.chromaDelayHorizontal,
                vhs?.chromaLoss ?: 0f,
                vhsSharpen?.intensity ?: 0f,
                vhsSharpen?.frequency ?: 0f,
                vhsEdgeWave?.intensity ?: 0f,
                vhsEdgeWave?.speed ?: 0f,
                vhsEdgeWave?.frequency ?: 0f,
                scale?.horizontal ?: 1f,
                scale?.vertical ?: 1f
            ),
            booleanSettings = booleanArrayOf(
                headSwitching != null,
                headSwitchingMidLine != null,
                trackingNoise != null,
                compositeNoise != null,
                ringing != null,
                lumaNoise != null,
                chromaNoise != null,
                vhs != null,
                vhsSharpen != null,
                vhsEdgeWave != null,
                settings.chromaVertBlend,
                scale != null,
                scale?.scaleWithVideoSize ?: false
            )
        ) ?: src
    }

    private external fun ntscImpl(
        input: Bitmap,
        frame: Int,
        scaleFactorX: Float,
        scaleFactorY: Float,
        intSettings: IntArray,
        floatSettings: FloatArray,
        booleanSettings: BooleanArray
    ): Bitmap?

    override suspend fun blockGlitch(
        src: Bitmap,
        strength: Float,
        blockSizeFraction: Float
    ): Bitmap = runInterruptible {
        blockGlitchImpl(
            src = src.safe(),
            strength = strength,
            blockSizeFraction = blockSizeFraction
        ) ?: src
    }

    private external fun blockGlitchImpl(
        src: Bitmap,
        strength: Float,
        blockSizeFraction: Float
    ): Bitmap?

    override suspend fun crtCurvature(
        src: Bitmap,
        curvature: Float,
        vignette: Float,
        chroma: Float
    ): Bitmap = runInterruptible {
        crtCurvatureImpl(
            src = src.safe(),
            curvature = curvature,
            vignette = vignette,
            chroma = chroma
        ) ?: src
    }

    private external fun crtCurvatureImpl(
        src: Bitmap,
        curvature: Float,
        vignette: Float,
        chroma: Float
    ): Bitmap?

    override suspend fun pixelMelt(
        src: Bitmap,
        strength: Float,
        maxDrop: Int
    ): Bitmap = runInterruptible {
        pixelMeltImpl(
            src = src.safe(),
            strength = strength,
            maxDrop = maxDrop
        ) ?: src
    }

    override suspend fun bloom(
        src: Bitmap,
        threshold: Float,
        intensity: Float,
        radius: Int,
        softKnee: Float,
        exposure: Float,
        gamma: Float
    ): Bitmap = runInterruptible {
        bloomImpl(
            input = src.safe(),
            threshold = threshold,
            intensity = intensity,
            radius = radius,
            softKnee = softKnee,
            exposure = exposure,
            gamma = gamma
        ) ?: src
    }

    override suspend fun autoWhiteBalance(
        input: Bitmap,
        strength: Float,
        clipPercent: Float
    ): Bitmap = runInterruptible {
        autoWhiteBalanceImpl(
            input = input.safe(),
            strength = strength,
            clipPercent = clipPercent
        ) ?: input
    }

    private external fun bloomImpl(
        input: Bitmap,
        threshold: Float,
        intensity: Float,
        radius: Int,
        softKnee: Float,
        exposure: Float,
        gamma: Float
    ): Bitmap?

    private external fun autoWhiteBalanceImpl(
        input: Bitmap,
        strength: Float,
        clipPercent: Float
    ): Bitmap?

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
