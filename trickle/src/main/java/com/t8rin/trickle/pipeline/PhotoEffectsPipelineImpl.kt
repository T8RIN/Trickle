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

    override suspend fun heightAmbientOcclusion(
        input: Bitmap,
        radius: Float,
        depth: Float,
        directions: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        heightAmbientOcclusionImpl(
            input = source,
            radius = radius.bounded(0.005f, 0.2f, 0.045f),
            depth = depth.bounded(0f, 5f, 1.8f),
            directions = directions.coerceIn(4, 16),
            strength = amount
        )
    }

    override suspend fun heightCastShadows(
        input: Bitmap,
        azimuth: Float,
        elevation: Float,
        depth: Float,
        softness: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        heightCastShadowsImpl(
            input = source,
            azimuth = azimuth.bounded(0f, 360f, 135f),
            elevation = elevation.bounded(5f, 85f, 35f),
            depth = depth.bounded(0f, 1f, 0.35f),
            softness = softness.bounded(0f, 1f, 0.15f),
            strength = amount
        )
    }

    override suspend fun refractiveCaustics(
        input: Bitmap,
        scale: Float,
        refraction: Float,
        dispersion: Float,
        focus: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        refractiveCausticsImpl(
            input = source,
            scale = scale.bounded(0.2f, 4f, 1f),
            refraction = refraction.bounded(0f, 0.12f, 0.035f),
            dispersion = dispersion.bounded(0f, 1f, 0.25f),
            focus = focus.bounded(0.2f, 3f, 1.4f),
            strength = amount
        )
    }

    override suspend fun thinFilmIridescence(
        input: Bitmap,
        thickness: Float,
        variation: Float,
        angle: Float,
        saturation: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        thinFilmIridescenceImpl(
            input = source,
            thickness = thickness.bounded(80f, 1400f, 480f),
            variation = variation.bounded(0f, 1200f, 320f),
            angle = angle.bounded(0f, 80f, 35f),
            saturation = saturation.bounded(0f, 1f, 0.8f),
            strength = amount
        )
    }

    override suspend fun diffractionGrating(
        input: Bitmap,
        spacing: Float,
        angle: Float,
        relief: Float,
        bandwidth: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        diffractionGratingImpl(
            input = source,
            spacing = spacing.bounded(0.7f, 5f, 1.6f),
            angle = angle.bounded(0f, 180f, 25f),
            relief = relief.bounded(0f, 2f, 0.65f),
            bandwidth = bandwidth.bounded(0.1f, 1f, 0.35f),
            strength = amount
        )
    }

    override suspend fun flowTexture(
        input: Bitmap,
        length: Float,
        detail: Float,
        contrast: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        flowTextureImpl(
            input = source,
            length = length.bounded(0.005f, 0.1f, 0.035f),
            detail = detail.bounded(0.3f, 3f, 1.4f),
            contrast = contrast.bounded(0f, 2f, 0.7f),
            seed = seed.coerceIn(0, 10000),
            strength = amount
        )
    }

    override suspend fun hilbertRibbon(
        input: Bitmap,
        order: Int,
        lineWidth: Float,
        contrast: Float,
        ink: Int,
        paper: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        hilbertRibbonImpl(
            input = source,
            order = order.coerceIn(2, 9),
            lineWidth = lineWidth.bounded(0.1f, 1f, 0.85f),
            contrast = contrast.bounded(0.25f, 3f, 2.5f),
            ink = ink,
            paper = paper,
            strength = amount
        )
    }

    override suspend fun mazePortrait(
        input: Bitmap,
        cellSize: Float,
        wallWidth: Float,
        bias: Float,
        seed: Int,
        ink: Int,
        paper: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        mazePortraitImpl(
            input = source,
            cellSize = cellSize.bounded(4f, 64f, 12f),
            wallWidth = wallWidth.bounded(0.05f, 0.48f, 0.46f),
            bias = bias.bounded(0f, 8f, 3f),
            seed = seed.coerceIn(0, 10000),
            ink = ink,
            paper = paper,
            strength = amount
        )
    }

    override suspend fun knittedFabric(
        input: Bitmap,
        stitchSize: Float,
        yarnWidth: Float,
        relief: Float,
        twist: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        knittedFabricImpl(
            input = source,
            stitchSize = stitchSize.bounded(6f, 64f, 16f),
            yarnWidth = yarnWidth.bounded(0.08f, 0.34f, 0.2f),
            relief = relief.bounded(0f, 1f, 0.7f),
            twist = twist.bounded(0f, 1f, 0.6f),
            strength = amount
        )
    }

    override suspend fun origamiRelief(
        input: Bitmap,
        cellSize: Float,
        foldHeight: Float,
        lightAngle: Float,
        crease: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        origamiReliefImpl(
            input = source,
            cellSize = cellSize.bounded(16f, 160f, 72f),
            foldHeight = foldHeight.bounded(0f, 1f, 0.5f),
            lightAngle = lightAngle.bounded(0f, 360f, 315f),
            crease = crease.bounded(0f, 1f, 0.45f),
            strength = amount
        )
    }

    override suspend fun pageCurl(
        input: Bitmap,
        amount: Float,
        radius: Float,
        angle: Float,
        shadow: Float,
        backside: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, blend ->
        pageCurlImpl(
            input = source,
            amount = amount.bounded(0f, 0.75f, 0.25f),
            radius = radius.bounded(0.015f, 0.2f, 0.08f),
            angle = angle.bounded(0f, 90f, 35f),
            shadow = shadow.bounded(0f, 1f, 0.6f),
            backside = backside,
            strength = blend
        )
    }

    override suspend fun curlAdvection(
        input: Bitmap,
        scale: Float,
        distance: Float,
        steps: Int,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        curlAdvectionImpl(
            input = source,
            scale = scale.bounded(0.5f, 12f, 4f),
            distance = distance.bounded(0f, 160f, 48f),
            steps = steps.coerceIn(4, 48),
            seed = seed.coerceIn(0, 10000),
            strength = amount
        )
    }

    override suspend fun histogramMosaic(
        input: Bitmap,
        cells: Int,
        bins: Int,
        gap: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        histogramMosaicImpl(
            input = source,
            cells = cells.coerceIn(4, 64),
            bins = bins.coerceIn(4, 16),
            gap = gap.bounded(0f, 0.3f, 0.08f),
            strength = amount
        )
    }

    override suspend fun localBinaryPattern(
        input: Bitmap,
        radius: Float,
        tolerance: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        localBinaryPatternImpl(
            input = source,
            radius = radius.bounded(1f, 16f, 3f),
            tolerance = tolerance.bounded(0f, 0.2f, 0.02f),
            strength = amount
        )
    }

    override suspend fun localEntropyMap(
        input: Bitmap,
        radius: Float,
        bins: Int,
        gain: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        localEntropyMapImpl(
            input = source,
            radius = radius.bounded(1f, 64f, 12f),
            bins = bins.coerceIn(4, 32),
            gain = gain.bounded(0.25f, 3f, 1f),
            strength = amount
        )
    }

    override suspend fun nailStringArt(
        input: Bitmap,
        nails: Int,
        threads: Int,
        threadWidth: Float,
        contrast: Float,
        ink: Int,
        paper: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        nailStringArtImpl(
            input = source,
            nails = nails.coerceIn(32, 256),
            threads = threads.coerceIn(100, 3000),
            threadWidth = threadWidth.bounded(0.3f, 2f, 0.8f),
            contrast = contrast.bounded(0.25f, 3f, 1f),
            ink = ink,
            paper = paper,
            strength = amount
        )
    }

    override suspend fun foldedBook(
        input: Bitmap,
        pages: Int,
        depth: Float,
        viewAngle: Float,
        paper: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        foldedBookImpl(
            input = source,
            pages = pages.coerceIn(24, 256),
            depth = depth.bounded(0f, 128f, 48f),
            viewAngle = viewAngle.bounded(-60f, 60f, 25f),
            paper = paper,
            strength = amount
        )
    }

    override suspend fun paperQuilling(
        input: Bitmap,
        cellSize: Float,
        turns: Float,
        stripWidth: Float,
        relief: Float,
        background: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        paperQuillingImpl(
            input = source,
            cellSize = cellSize.bounded(12f, 96f, 40f),
            turns = turns.bounded(1f, 8f, 3.5f),
            stripWidth = stripWidth.bounded(0.025f, 0.18f, 0.06f),
            relief = relief.bounded(0f, 1f, 0.8f),
            background = background,
            strength = amount
        )
    }

    override suspend fun lenticularPrint(
        input: Bitmap,
        lensWidth: Float,
        viewAngle: Float,
        refraction: Float,
        gloss: Float,
        rotation: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        lenticularPrintImpl(
            input = source,
            lensWidth = lensWidth.bounded(4f, 64f, 16f),
            viewAngle = viewAngle.bounded(-50f, 50f, 0f),
            refraction = refraction.bounded(1f, 2.2f, 1.5f),
            gloss = gloss.bounded(0f, 1f, 0.45f),
            rotation = rotation.bounded(-90f, 90f, 0f),
            strength = amount
        )
    }

    override suspend fun pinArt(
        input: Bitmap,
        spacing: Float,
        height: Float,
        angle: Float,
        gloss: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        pinArtImpl(
            input = source,
            spacing = spacing.bounded(5f, 40f, 18f),
            height = height.bounded(0.005f, 0.2f, 0.065f),
            angle = angle.bounded(0f, 70f, 35f),
            gloss = gloss.bounded(0f, 1f, 0.65f),
            strength = amount
        )
    }

    override suspend fun ceramicGlaze(
        input: Bitmap,
        size: Float,
        pooling: Float,
        gloss: Float,
        crackle: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        ceramicGlazeImpl(
            input = source,
            size = size.bounded(12f, 96f, 32f),
            pooling = pooling.bounded(0f, 1f, 0.55f),
            gloss = gloss.bounded(0f, 1f, 0.7f),
            crackle = crackle.bounded(0f, 1f, 0.5f),
            seed = seed.coerceIn(0, 10000),
            strength = amount
        )
    }

    override suspend fun marquetry(
        input: Bitmap,
        size: Float,
        grain: Float,
        bevel: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        marquetryImpl(
            input = source,
            size = size.bounded(12f, 96f, 42f),
            grain = grain.bounded(0f, 1f, 0.7f),
            bevel = bevel.bounded(0f, 1f, 0.65f),
            seed = seed.coerceIn(0, 10000),
            strength = amount
        )
    }

    override suspend fun photoelasticStress(
        input: Bitmap,
        load: Float,
        smoothing: Float,
        angle: Float,
        ambient: Float,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        photoelasticStressImpl(
            input = source,
            load = load.bounded(0.2f, 8f, 2.4f),
            smoothing = smoothing.bounded(1f, 24f, 8f),
            angle = angle.bounded(0f, 180f, 25f),
            ambient = ambient.bounded(0f, 1f, 0.22f),
            strength = amount
        )
    }

    override suspend fun condensation(
        input: Bitmap,
        size: Float,
        coverage: Float,
        refraction: Float,
        haze: Float,
        seed: Int,
        strength: Float
    ): Bitmap = apply(input, strength) { source, amount ->
        condensationImpl(
            input = source,
            size = size.bounded(8f, 80f, 48f),
            coverage = coverage.bounded(0f, 1f, 0.65f),
            refraction = refraction.bounded(0f, 1.5f, 0.7f),
            haze = haze.bounded(0f, 0.6f, 0.18f),
            seed = seed.coerceIn(0, 10000),
            strength = amount
        )
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

    private external fun heightAmbientOcclusionImpl(
        input: Bitmap,
        radius: Float,
        depth: Float,
        directions: Int,
        strength: Float
    ): Bitmap?

    private external fun heightCastShadowsImpl(
        input: Bitmap,
        azimuth: Float,
        elevation: Float,
        depth: Float,
        softness: Float,
        strength: Float
    ): Bitmap?

    private external fun refractiveCausticsImpl(
        input: Bitmap,
        scale: Float,
        refraction: Float,
        dispersion: Float,
        focus: Float,
        strength: Float
    ): Bitmap?

    private external fun thinFilmIridescenceImpl(
        input: Bitmap,
        thickness: Float,
        variation: Float,
        angle: Float,
        saturation: Float,
        strength: Float
    ): Bitmap?

    private external fun diffractionGratingImpl(
        input: Bitmap,
        spacing: Float,
        angle: Float,
        relief: Float,
        bandwidth: Float,
        strength: Float
    ): Bitmap?

    private external fun flowTextureImpl(
        input: Bitmap,
        length: Float,
        detail: Float,
        contrast: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun hilbertRibbonImpl(
        input: Bitmap,
        order: Int,
        lineWidth: Float,
        contrast: Float,
        ink: Int,
        paper: Int,
        strength: Float
    ): Bitmap?

    private external fun mazePortraitImpl(
        input: Bitmap,
        cellSize: Float,
        wallWidth: Float,
        bias: Float,
        seed: Int,
        ink: Int,
        paper: Int,
        strength: Float
    ): Bitmap?

    private external fun knittedFabricImpl(
        input: Bitmap,
        stitchSize: Float,
        yarnWidth: Float,
        relief: Float,
        twist: Float,
        strength: Float
    ): Bitmap?

    private external fun origamiReliefImpl(
        input: Bitmap,
        cellSize: Float,
        foldHeight: Float,
        lightAngle: Float,
        crease: Float,
        strength: Float
    ): Bitmap?

    private external fun pageCurlImpl(
        input: Bitmap,
        amount: Float,
        radius: Float,
        angle: Float,
        shadow: Float,
        backside: Int,
        strength: Float
    ): Bitmap?

    private external fun curlAdvectionImpl(
        input: Bitmap,
        scale: Float,
        distance: Float,
        steps: Int,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun histogramMosaicImpl(
        input: Bitmap,
        cells: Int,
        bins: Int,
        gap: Float,
        strength: Float
    ): Bitmap?

    private external fun localBinaryPatternImpl(
        input: Bitmap,
        radius: Float,
        tolerance: Float,
        strength: Float
    ): Bitmap?

    private external fun localEntropyMapImpl(
        input: Bitmap,
        radius: Float,
        bins: Int,
        gain: Float,
        strength: Float
    ): Bitmap?

    private external fun nailStringArtImpl(
        input: Bitmap,
        nails: Int,
        threads: Int,
        threadWidth: Float,
        contrast: Float,
        ink: Int,
        paper: Int,
        strength: Float
    ): Bitmap?

    private external fun foldedBookImpl(
        input: Bitmap,
        pages: Int,
        depth: Float,
        viewAngle: Float,
        paper: Int,
        strength: Float
    ): Bitmap?

    private external fun paperQuillingImpl(
        input: Bitmap,
        cellSize: Float,
        turns: Float,
        stripWidth: Float,
        relief: Float,
        background: Int,
        strength: Float
    ): Bitmap?

    private external fun lenticularPrintImpl(
        input: Bitmap,
        lensWidth: Float,
        viewAngle: Float,
        refraction: Float,
        gloss: Float,
        rotation: Float,
        strength: Float
    ): Bitmap?

    private external fun pinArtImpl(
        input: Bitmap,
        spacing: Float,
        height: Float,
        angle: Float,
        gloss: Float,
        strength: Float
    ): Bitmap?

    private external fun ceramicGlazeImpl(
        input: Bitmap,
        size: Float,
        pooling: Float,
        gloss: Float,
        crackle: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun marquetryImpl(
        input: Bitmap,
        size: Float,
        grain: Float,
        bevel: Float,
        seed: Int,
        strength: Float
    ): Bitmap?

    private external fun photoelasticStressImpl(
        input: Bitmap,
        load: Float,
        smoothing: Float,
        angle: Float,
        ambient: Float,
        strength: Float
    ): Bitmap?

    private external fun condensationImpl(
        input: Bitmap,
        size: Float,
        coverage: Float,
        refraction: Float,
        haze: Float,
        seed: Int,
        strength: Float
    ): Bitmap?
}
