package com.t8rin.trickle.pipeline

import android.graphics.Bitmap
import com.t8rin.trickle.DitheringPipeline
import com.t8rin.trickle.DitheringType

internal object DitheringPipelineImpl : DitheringPipeline {

    override fun dithering(
        input: Bitmap,
        type: DitheringType,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = when (type) {
        DitheringType.BayerTwo -> ordered2By2BayerDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.BayerThree -> ordered3By3BayerDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.BayerFour -> ordered4By4BayerDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.BayerEight -> ordered8By8BayerDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.FloydSteinberg -> floydSteinbergDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.JarvisJudiceNinke -> jarvisJudiceNinkeDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Sierra -> sierraDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.TwoRowSierra -> twoRowSierraDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.SierraLite -> sierraLiteDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Atkinson -> atkinsonDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Stucki -> stuckiDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Burkes -> burkesDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.FalseFloydSteinberg -> falseFloydSteinbergDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.LeftToRight -> simpleLeftToRightErrorDiffusionDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Random -> randomDithering(
            input = input,
            isGrayScale = isGrayScale
        )

        DitheringType.SimpleThreshold -> simpleThresholdDithering(
            input = input,
            threshold = threshold,
            isGrayScale = isGrayScale
        )
    }

    override fun ordered2By2BayerDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = ordered2By2BayerImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun ordered3By3BayerDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = ordered3By3BayerImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun ordered4By4BayerDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = ordered4By4BayerImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun ordered8By8BayerDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = ordered8By8BayerImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun floydSteinbergDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = floydSteinbergImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun jarvisJudiceNinkeDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = jarvisJudiceNinkeImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun sierraDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = sierraImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun twoRowSierraDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = twoRowSierraImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun sierraLiteDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = sierraLiteImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun atkinsonDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = atkinsonImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun stuckiDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = stuckiImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun burkesDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = burkesImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun falseFloydSteinbergDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = falseFloydSteinbergImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun simpleLeftToRightErrorDiffusionDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = simpleLeftToRightErrorDiffusionImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    override fun randomDithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap = randomImpl(
        input = input,
        isGrayScale = isGrayScale
    ) ?: input

    override fun simpleThresholdDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = simpleThresholdImpl(
        input = input,
        threshold = threshold,
        isGrayScale = isGrayScale
    ) ?: input

    private external fun ordered2By2BayerImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun ordered3By3BayerImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun ordered4By4BayerImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun ordered8By8BayerImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun floydSteinbergImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun jarvisJudiceNinkeImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun sierraImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun twoRowSierraImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun sierraLiteImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun atkinsonImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun stuckiImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun burkesImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun falseFloydSteinbergImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun simpleLeftToRightErrorDiffusionImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun randomImpl(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun simpleThresholdImpl(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?
}
