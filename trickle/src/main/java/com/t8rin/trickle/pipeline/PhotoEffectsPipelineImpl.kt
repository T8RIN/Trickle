package com.t8rin.trickle.pipeline

import android.graphics.Bitmap
import com.t8rin.trickle.PhotoEffectsPipeline
import com.t8rin.trickle.TrickleUtils.safe
import kotlinx.coroutines.runInterruptible

internal object PhotoEffectsPipelineImpl : NativeLoader(), PhotoEffectsPipeline {

    override suspend fun palComposite(
        input: Bitmap,
        lumaFilter: Float,
        chromaRadius: Int,
        noise: Float,
        phaseError: Int,
        scanSamples: Int,
        lineCompensation: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        palCompositeImpl(
            input = source,
            lumaFilter = lumaFilter.bounded(0f, 1f, 0.88f),
            chromaRadius = chromaRadius.coerceIn(1, 6),
            noise = noise.bounded(0f, 0.3f, 0.035f),
            phaseError = phaseError.coerceIn(-45, 45),
            scanSamples = scanSamples.coerceIn(160, 960),
            lineCompensation = lineCompensation.bounded(0f, 1f, 0.7f),
            seed = seed,
            strength = amount
        )
    }

    override suspend fun secam(
        input: Bitmap,
        chromaRadius: Int,
        frequencyDeviation: Float,
        noise: Float,
        preEmphasis: Float,
        lineCrosstalk: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        secamImpl(
            input = source,
            chromaRadius = chromaRadius.coerceIn(1, 16),
            frequencyDeviation = frequencyDeviation.bounded(0.2f, 1f, 0.8f),
            noise = noise.bounded(0f, 0.65f, 0.12f),
            preEmphasis = preEmphasis.bounded(0f, 1f, 0.55f),
            lineCrosstalk = lineCrosstalk.bounded(0f, 0.5f, 0.07f),
            seed = seed,
            strength = amount
        )
    }

    override suspend fun rfReception(
        input: Bitmap,
        delay: Int,
        reflection: Float,
        noise: Float,
        frequency: Int,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        rfReceptionImpl(
            input = source,
            delay = delay.coerceIn(1, 85),
            reflection = reflection.bounded(0f, 0.9f, 0.42f),
            noise = noise.bounded(0f, 0.4f, 0.09f),
            frequency = frequency.coerceIn(1, 120),
            seed = seed,
            strength = amount
        )
    }

    override suspend fun sstvReception(
        input: Bitmap,
        transmissionMode: Int,
        frequencyDrift: Float,
        syncError: Float,
        noise: Float,
        dropout: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        sstvReceptionImpl(
            input = source,
            transmissionMode = transmissionMode.coerceIn(0, 1),
            frequencyDrift = frequencyDrift.bounded(0f, 0.25f, 0.06f),
            syncError = syncError.bounded(0f, 0.08f, 0.014f),
            noise = noise.bounded(0f, 0.4f, 0.08f),
            dropout = dropout.bounded(0f, 0.2f, 0.035f),
            seed = seed,
            strength = amount
        )
    }

    override suspend fun lcdSubpixels(
        input: Bitmap,
        size: Int,
        blackMatrix: Float,
        subpixelOrder: Int,
        viewAngle: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        lcdSubpixelsImpl(
            input = source,
            size = size.coerceIn(3, 36),
            blackMatrix = blackMatrix.bounded(0.02f, 0.6f, 0.17f),
            subpixelOrder = subpixelOrder.coerceIn(0, 1),
            viewAngle = viewAngle.coerceIn(-70, 70),
            strength = amount
        )
    }

    override suspend fun filmGrain(
        input: Bitmap,
        size: Float,
        intensity: Float,
        sizeVariation: Float,
        exposureDependence: Float,
        colorGrain: Float,
        seed: Int
    ): Bitmap = apply(input) { source, _ ->
        filmGrainImpl(
            input = source,
            size = size.bounded(0.65f, 12f, 2.1f),
            intensity = intensity.bounded(0f, 0.7f, 0.23f),
            sizeVariation = sizeVariation.bounded(0f, 1f, 0.6f),
            exposureDependence = exposureDependence.bounded(0f, 1f, 0.72f),
            colorGrain = colorGrain.bounded(0f, 1f, 0.13f),
            seed = seed
        )
    }

    override suspend fun filmNegative(
        input: Bitmap,
        exposure: Float,
        densityRange: Float,
        contrast: Float,
        toneCurve: Float,
        filmBase: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        filmNegativeImpl(
            input = source,
            exposure = exposure.bounded(-3f, 3f, 0.65f),
            densityRange = densityRange.bounded(0.2f, 6f, 3f),
            contrast = contrast.bounded(0.4f, 2.4f, 1f),
            toneCurve = toneCurve.bounded(0f, 1f, 0.35f),
            filmBase = filmBase,
            strength = amount
        )
    }

    override suspend fun agxToneMapping(
        input: Bitmap,
        exposure: Float,
        look: Int,
        saturation: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        agxToneMappingImpl(
            input = source,
            exposure = exposure.bounded(-4f, 4f, 0f),
            look = look.coerceIn(0, 2),
            saturation = saturation.bounded(0f, 1.75f, 1f),
            strength = amount
        )
    }

    override suspend fun waveletDenoise(
        input: Bitmap,
        lumaNoise: Float,
        chromaNoise: Float,
        levels: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        waveletDenoiseImpl(
            input = source,
            lumaNoise = lumaNoise.bounded(0f, 0.15f, 0.018f),
            chromaNoise = chromaNoise.bounded(0f, 0.15f, 0.03f),
            levels = levels.coerceIn(1, 4),
            strength = amount
        )
    }

    override suspend fun daltonization(
        input: Bitmap,
        visionType: Int,
        severity: Float,
        correction: Float,
        preserveLuminance: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        daltonizationImpl(
            input = source,
            visionType = visionType.coerceIn(0, 2),
            severity = severity.bounded(0f, 1f, 1f),
            correction = correction.bounded(0f, 1.5f, 0.8f),
            preserveLuminance = preserveLuminance.bounded(0f, 1f, 0.8f),
            strength = amount
        )
    }

    override suspend fun retroHardware(
        input: Bitmap,
        system: Int,
        scale: Float,
        dithering: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        retroHardwareImpl(
            input = source,
            system = system.coerceIn(0, 1),
            scale = scale.bounded(1f, 4f, 1f),
            dithering = dithering.bounded(0f, 1f, 0.25f),
            strength = amount
        )
    }

    override suspend fun risograph(
        input: Bitmap,
        spacing: Int,
        misregistration: Float,
        dotGain: Float,
        inks: Int,
        paperGrain: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        risographImpl(
            input = source,
            spacing = spacing.coerceIn(2, 30),
            misregistration = misregistration.bounded(0f, 20f, 2.5f),
            dotGain = dotGain.bounded(0f, 1f, 0.32f),
            inks = inks.coerceIn(0, 2),
            paperGrain = paperGrain.bounded(0f, 1f, 0.4f),
            seed = seed,
            strength = amount
        )
    }

    override suspend fun cmykPress(
        input: Bitmap,
        spacing: Int,
        blackGeneration: Float,
        dotGain: Float,
        misregistration: Float,
        inkLimit: Float,
        paperGrain: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        cmykPressImpl(
            input = source,
            spacing = spacing.coerceIn(2, 30),
            blackGeneration = blackGeneration.bounded(0f, 1f, 0.68f),
            dotGain = dotGain.bounded(0f, 1f, 0.22f),
            misregistration = misregistration.bounded(0f, 18f, 1.2f),
            inkLimit = inkLimit.bounded(0.75f, 4f, 2.6f),
            paperGrain = paperGrain.bounded(0f, 1f, 0.4f),
            seed = seed,
            strength = amount
        )
    }

    override suspend fun thermalReceipt(
        input: Bitmap,
        size: Int,
        heat: Float,
        heatRetention: Float,
        defects: Float,
        fading: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        thermalReceiptImpl(
            input = source,
            size = size.coerceIn(1, 22),
            heat = heat.bounded(0.4f, 2.5f, 1.35f),
            heatRetention = heatRetention.bounded(0f, 0.9f, 0.32f),
            defects = defects.bounded(0f, 0.15f, 0.015f),
            fading = fading.bounded(0f, 0.8f, 0.1f),
            seed = seed,
            strength = amount
        )
    }

    override suspend fun photoCrossStitch(
        input: Bitmap,
        size: Int,
        threadColors: Int,
        threadWidth: Float,
        lightAngle: Int,
        relief: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        photoCrossStitchImpl(
            input = source,
            size = size.coerceIn(4, 38),
            threadColors = threadColors.coerceIn(2, 64),
            threadWidth = threadWidth.bounded(0.05f, 0.25f, 0.12f),
            lightAngle = lightAngle.coerceIn(0, 360),
            relief = relief.bounded(0f, 1f, 0.7f),
            seed = seed,
            strength = amount
        )
    }

    override suspend fun autostereogram(
        input: Bitmap,
        repeatDistance: Float,
        depth: Float,
        gamma: Float,
        size: Int,
        softness: Float,
        invert: Int,
        seed: Int
    ): Bitmap = apply(input) { source, _ ->
        autostereogramImpl(
            input = source,
            repeatDistance = repeatDistance.bounded(0.06f, 0.3f, 0.15f),
            depth = depth.bounded(0.02f, 0.65f, 0.34f),
            gamma = gamma.bounded(0.4f, 2.5f, 1f),
            size = size.coerceIn(1, 7),
            softness = softness.bounded(0f, 16f, 3f),
            invert = invert.coerceIn(0, 1),
            seed = seed
        )
    }

    override suspend fun paletteRemap(
        input: Bitmap,
        palette: IntArray,
        dithering: Float
    ): Bitmap {
        require(palette.size in 1..7) { "The palette must contain between 1 and 7 colors" }
        return apply(input) { source, _ ->
            paletteRemapImpl(source, palette, dithering.bounded(0f, 0.5f, 0.12f))
        }
    }

    private suspend fun apply(
        input: Bitmap,
        strength: Float = 1f,
        filter: (Bitmap, Float) -> Bitmap?
    ): Bitmap = runInterruptible {
        val amount = strength.bounded(0f, 1f, 1f)
        if (amount == 0f) return@runInterruptible input
        val source = input.safe()
        try {
            filter(source, amount)?.apply {
                density = input.density
            } ?: input
        } finally {
            if (source !== input) source.recycle()
        }
    }

    private fun Float.bounded(min: Float, max: Float, default: Float): Float =
        if (isFinite()) coerceIn(min, max) else default

    private external fun palCompositeImpl(
        input: Bitmap,
        lumaFilter: Float,
        chromaRadius: Int,
        noise: Float,
        phaseError: Int,
        scanSamples: Int,
        lineCompensation: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun secamImpl(
        input: Bitmap,
        chromaRadius: Int,
        frequencyDeviation: Float,
        noise: Float,
        preEmphasis: Float,
        lineCrosstalk: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun rfReceptionImpl(
        input: Bitmap,
        delay: Int,
        reflection: Float,
        noise: Float,
        frequency: Int,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun sstvReceptionImpl(
        input: Bitmap,
        transmissionMode: Int,
        frequencyDrift: Float,
        syncError: Float,
        noise: Float,
        dropout: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun lcdSubpixelsImpl(
        input: Bitmap,
        size: Int,
        blackMatrix: Float,
        subpixelOrder: Int,
        viewAngle: Int,
        strength: Float
    ): Bitmap?

    private external fun filmGrainImpl(
        input: Bitmap,
        size: Float,
        intensity: Float,
        sizeVariation: Float,
        exposureDependence: Float,
        colorGrain: Float,
        seed: Int
    ): Bitmap?

    private external fun filmNegativeImpl(
        input: Bitmap,
        exposure: Float,
        densityRange: Float,
        contrast: Float,
        toneCurve: Float,
        filmBase: Int,
        strength: Float
    ): Bitmap?

    private external fun agxToneMappingImpl(
        input: Bitmap,
        exposure: Float,
        look: Int,
        saturation: Float,
        strength: Float
    ): Bitmap?

    private external fun waveletDenoiseImpl(
        input: Bitmap,
        lumaNoise: Float,
        chromaNoise: Float,
        levels: Int,
        strength: Float
    ): Bitmap?

    private external fun daltonizationImpl(
        input: Bitmap,
        visionType: Int,
        severity: Float,
        correction: Float,
        preserveLuminance: Float,
        strength: Float
    ): Bitmap?

    private external fun retroHardwareImpl(
        input: Bitmap,
        system: Int,
        scale: Float,
        dithering: Float,
        strength: Float
    ): Bitmap?

    private external fun risographImpl(
        input: Bitmap,
        spacing: Int,
        misregistration: Float,
        dotGain: Float,
        inks: Int,
        paperGrain: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun cmykPressImpl(
        input: Bitmap,
        spacing: Int,
        blackGeneration: Float,
        dotGain: Float,
        misregistration: Float,
        inkLimit: Float,
        paperGrain: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun thermalReceiptImpl(
        input: Bitmap,
        size: Int,
        heat: Float,
        heatRetention: Float,
        defects: Float,
        fading: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun photoCrossStitchImpl(
        input: Bitmap,
        size: Int,
        threadColors: Int,
        threadWidth: Float,
        lightAngle: Int,
        relief: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun autostereogramImpl(
        input: Bitmap,
        repeatDistance: Float,
        depth: Float,
        gamma: Float,
        size: Int,
        softness: Float,
        invert: Int,
        seed: Int
    ): Bitmap?

    private external fun paletteRemapImpl(
        input: Bitmap,
        palette: IntArray,
        dithering: Float
    ): Bitmap?
}
