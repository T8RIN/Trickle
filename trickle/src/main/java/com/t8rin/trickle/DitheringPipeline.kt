package com.t8rin.trickle

import android.graphics.Bitmap

interface DitheringPipeline {

    suspend fun dithering(
        input: Bitmap,
        type: DitheringType,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun ordered2By2BayerDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun ordered3By3BayerDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun ordered4By4BayerDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun ordered8By8BayerDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun floydSteinbergDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun jarvisJudiceNinkeDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun sierraDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun twoRowSierraDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun sierraLiteDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun atkinsonDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun stuckiDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun burkesDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun falseFloydSteinbergDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun simpleLeftToRightErrorDiffusionDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun randomDithering(
        input: Bitmap,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun simpleThresholdDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    suspend fun clustered2x2Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap

    suspend fun clustered4x4Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap

    suspend fun clustered8x8Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap

    suspend fun ylilomaDithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap

}