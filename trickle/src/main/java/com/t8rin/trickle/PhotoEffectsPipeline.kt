package com.t8rin.trickle

import android.graphics.Bitmap

interface PhotoEffectsPipeline {

    suspend fun palComposite(
        input: Bitmap,
        lumaFilter: Float = 0.88f,
        chromaRadius: Int = 2,
        noise: Float = 0.035f,
        phaseError: Int = 12,
        scanSamples: Int = 720,
        lineCompensation: Float = 0.7f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    suspend fun secam(
        input: Bitmap,
        chromaRadius: Int = 4,
        frequencyDeviation: Float = 0.8f,
        noise: Float = 0.12f,
        preEmphasis: Float = 0.55f,
        lineCrosstalk: Float = 0.07f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    suspend fun rfReception(
        input: Bitmap,
        delay: Int = 19,
        reflection: Float = 0.42f,
        noise: Float = 0.09f,
        frequency: Int = 21,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    suspend fun sstvReception(
        input: Bitmap,
        transmissionMode: Int = 0,
        frequencyDrift: Float = 0.06f,
        syncError: Float = 0.014f,
        noise: Float = 0.08f,
        dropout: Float = 0.035f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    suspend fun lcdSubpixels(
        input: Bitmap,
        size: Int = 10,
        blackMatrix: Float = 0.17f,
        subpixelOrder: Int = 0,
        viewAngle: Int = 0,
        strength: Float = 0.9f
    ): Bitmap

    suspend fun filmGrain(
        input: Bitmap,
        size: Float = 2.1f,
        intensity: Float = 0.23f,
        sizeVariation: Float = 0.6f,
        exposureDependence: Float = 0.72f,
        colorGrain: Float = 0.13f,
        seed: Int = 0
    ): Bitmap

    suspend fun filmNegative(
        input: Bitmap,
        exposure: Float = 0.65f,
        densityRange: Float = 3f,
        contrast: Float = 1f,
        toneCurve: Float = 0.35f,
        filmBase: Int = 0xFFFFA352.toInt(),
        strength: Float = 1f
    ): Bitmap

    suspend fun agxToneMapping(
        input: Bitmap,
        exposure: Float = 0f,
        look: Int = 0,
        saturation: Float = 1f,
        strength: Float = 1f
    ): Bitmap

    suspend fun waveletDenoise(
        input: Bitmap,
        lumaNoise: Float = 0.018f,
        chromaNoise: Float = 0.03f,
        levels: Int = 3,
        strength: Float = 1f
    ): Bitmap

    suspend fun daltonization(
        input: Bitmap,
        visionType: Int = 1,
        severity: Float = 1f,
        correction: Float = 0.8f,
        preserveLuminance: Float = 0.8f,
        strength: Float = 1f
    ): Bitmap

    suspend fun retroHardware(
        input: Bitmap,
        system: Int = 0,
        scale: Float = 1f,
        dithering: Float = 0.25f,
        strength: Float = 1f
    ): Bitmap

    suspend fun risograph(
        input: Bitmap,
        spacing: Int = 8,
        misregistration: Float = 2.5f,
        dotGain: Float = 0.32f,
        inks: Int = 0,
        paperGrain: Float = 0.4f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    suspend fun cmykPress(
        input: Bitmap,
        spacing: Int = 8,
        blackGeneration: Float = 0.68f,
        dotGain: Float = 0.22f,
        misregistration: Float = 1.2f,
        inkLimit: Float = 2.6f,
        paperGrain: Float = 0.4f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    suspend fun thermalReceipt(
        input: Bitmap,
        size: Int = 4,
        heat: Float = 1.35f,
        heatRetention: Float = 0.32f,
        defects: Float = 0.015f,
        fading: Float = 0.1f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    suspend fun photoCrossStitch(
        input: Bitmap,
        size: Int = 11,
        threadColors: Int = 24,
        threadWidth: Float = 0.12f,
        lightAngle: Int = 35,
        relief: Float = 0.7f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    suspend fun autostereogram(
        input: Bitmap,
        repeatDistance: Float = 0.15f,
        depth: Float = 0.34f,
        gamma: Float = 1f,
        size: Int = 2,
        softness: Float = 3f,
        invert: Int = 0,
        seed: Int = 0
    ): Bitmap

    suspend fun paletteRemap(
        input: Bitmap,
        palette: IntArray,
        dithering: Float = 0.12f
    ): Bitmap
}
