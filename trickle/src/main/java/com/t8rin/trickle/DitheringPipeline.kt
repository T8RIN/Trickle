package com.t8rin.trickle

import android.graphics.Bitmap

interface DitheringPipeline {

    fun dithering(
        input: Bitmap,
        type: DitheringType,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun ordered2By2BayerDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun ordered3By3BayerDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun ordered4By4BayerDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun ordered8By8BayerDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun floydSteinbergDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun jarvisJudiceNinkeDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun sierraDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun twoRowSierraDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun sierraLiteDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun atkinsonDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun stuckiDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun burkesDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun falseFloydSteinbergDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun simpleLeftToRightErrorDiffusionDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun randomDithering(
        input: Bitmap,
        isGrayScale: Boolean = false
    ): Bitmap

    fun simpleThresholdDithering(
        input: Bitmap,
        threshold: Int = 128,
        isGrayScale: Boolean = false
    ): Bitmap

    fun clustered2x2Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap

    fun clustered4x4Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap

    fun clustered8x8Dithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap

    fun ylilomaDithering(
        input: Bitmap,
        isGrayScale: Boolean
    ): Bitmap

}