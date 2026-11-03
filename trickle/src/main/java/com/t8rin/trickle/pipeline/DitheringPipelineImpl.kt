package com.t8rin.trickle.pipeline

import android.graphics.Bitmap
import com.t8rin.trickle.DitheringPipeline
import com.t8rin.trickle.DitheringType
import com.t8rin.trickle.TrickleUtils.safe
import kotlinx.coroutines.runInterruptible

internal object DitheringPipelineImpl : DitheringPipeline {

    override suspend fun dithering(
        input: Bitmap,
        type: DitheringType,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = when (type) {
        DitheringType.BayerTwo -> ordered2By2BayerDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.BayerThree -> ordered3By3BayerDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.BayerFour -> ordered4By4BayerDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.BayerEight -> ordered8By8BayerDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.FloydSteinberg -> floydSteinbergDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.JarvisJudiceNinke -> jarvisJudiceNinkeDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Sierra -> sierraDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.TwoRowSierra -> twoRowSierraDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.SierraLite -> sierraLiteDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Atkinson -> atkinsonDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Stucki -> stuckiDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Burkes -> burkesDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.FalseFloydSteinberg -> falseFloydSteinbergDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.LeftToRight -> simpleLeftToRightErrorDiffusionDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Random -> randomDithering(
            input = input.safe(),
            isGrayScale = isGrayScale
        )

        DitheringType.SimpleThreshold -> simpleThresholdDithering(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        )

        DitheringType.Clustered2x2 -> clustered2x2Dithering(
            input = input.safe(),
            isGrayScale = isGrayScale
        )

        DitheringType.Clustered4x4 -> clustered4x4Dithering(
            input = input.safe(),
            isGrayScale = isGrayScale
        )

        DitheringType.Clustered8x8 -> clustered8x8Dithering(
            input = input.safe(),
            isGrayScale = isGrayScale
        )

        DitheringType.Yililoma -> ylilomaDithering(
            input = input.safe(),
            isGrayScale = isGrayScale
        )

        DitheringType.BayerSixteen,
        DitheringType.BlueNoise,
        DitheringType.InterleavedGradientNoise,
        DitheringType.ClusteredDot,
        DitheringType.Diamond,
        DitheringType.HorizontalLines,
        DitheringType.VerticalLines,
        DitheringType.DiagonalLines,
        DitheringType.CrosshatchMatrix,
        DitheringType.SerpentineFloydSteinberg,
        DitheringType.SerpentineAtkinson,
        DitheringType.SerpentineBurkes,
        DitheringType.Riemersma,
        DitheringType.Ostromoukhov,
        DitheringType.ShiauFan,
        DitheringType.SpiralHalftone,
        DitheringType.HexagonalDot,
        DitheringType.WaveLines -> runInterruptible {
            ditheringPatternImpl(
                input = input.safe(),
                pattern = type.ordinal - DitheringType.BayerSixteen.ordinal,
                threshold = threshold,
                isGrayScale = isGrayScale
            ) ?: input
        }
    }

    override suspend fun ordered2By2BayerDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        ordered2By2BayerImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun ordered3By3BayerDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        ordered3By3BayerImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun ordered4By4BayerDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        ordered4By4BayerImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun ordered8By8BayerDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        ordered8By8BayerImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun floydSteinbergDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        floydSteinbergImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun jarvisJudiceNinkeDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        jarvisJudiceNinkeImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun sierraDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        sierraImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun twoRowSierraDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        twoRowSierraImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun sierraLiteDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        sierraLiteImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun atkinsonDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        atkinsonImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun stuckiDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        stuckiImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun burkesDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        burkesImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun falseFloydSteinbergDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        falseFloydSteinbergImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun simpleLeftToRightErrorDiffusionDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        simpleLeftToRightErrorDiffusionImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun randomDithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        randomImpl(
            input = input.safe(),
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun simpleThresholdDithering(
        input: Bitmap,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        simpleThresholdImpl(
            input = input.safe(),
            threshold = threshold,
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun clustered2x2Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        clustered2x2DitheringImpl(
            input = input.safe(),
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun clustered4x4Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        clustered4x4DitheringImpl(
            input = input.safe(),
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun clustered8x8Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        clustered8x8DitheringImpl(
            input = input.safe(),
            isGrayScale = isGrayScale
        ) ?: input
    }

    override suspend fun ylilomaDithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap = runInterruptible {
        ylilomaDitheringImpl(
            input = input.safe(),
            isGrayScale = isGrayScale
        ) ?: input
    }

    private external fun ditheringPatternImpl(
        input: Bitmap,
        pattern: Int,
        threshold: Int,
        isGrayScale: Boolean
    ): Bitmap?

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

    private external fun clustered2x2DitheringImpl(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun clustered4x4DitheringImpl(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun clustered8x8DitheringImpl(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap?

    private external fun ylilomaDitheringImpl(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap?

}
