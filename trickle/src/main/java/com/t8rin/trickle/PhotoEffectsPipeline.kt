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

    /** Treat luminance as a height field and darken recesses according to their visible horizon. */
    suspend fun heightAmbientOcclusion(
        input: Bitmap,
        radius: Float = 0.045f,
        depth: Float = 1.8f,
        directions: Int = 8,
        strength: Float = 1f
    ): Bitmap

    /** Cast directional shadows across a relief derived from image luminance. */
    suspend fun heightCastShadows(
        input: Bitmap,
        azimuth: Float = 135f,
        elevation: Float = 35f,
        depth: Float = 0.35f,
        softness: Float = 0.15f,
        strength: Float = 1f
    ): Bitmap

    /** Focus refracted light through the image height field into bright caustic patterns. */
    suspend fun refractiveCaustics(
        input: Bitmap,
        scale: Float = 1f,
        refraction: Float = 0.035f,
        dispersion: Float = 0.25f,
        focus: Float = 1.4f,
        strength: Float = 1f
    ): Bitmap

    /** Create wavelength interference in a thin film whose thickness follows image luminance. */
    suspend fun thinFilmIridescence(
        input: Bitmap,
        thickness: Float = 480f,
        variation: Float = 320f,
        angle: Float = 35f,
        saturation: Float = 0.8f,
        strength: Float = 1f
    ): Bitmap

    /** Create angle-dependent spectral reflections from a relief diffraction grating. */
    suspend fun diffractionGrating(
        input: Bitmap,
        spacing: Float = 1.6f,
        angle: Float = 25f,
        relief: Float = 0.65f,
        bandwidth: Float = 0.35f,
        strength: Float = 1f
    ): Bitmap

    /** Convolve noise along the image tangent field to reveal continuous flowing textures. */
    suspend fun flowTexture(
        input: Bitmap,
        length: Float = 0.035f,
        detail: Float = 1.4f,
        contrast: Float = 0.7f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    /** Draw one continuous Hilbert ribbon with its width controlled by image darkness. */
    suspend fun hilbertRibbon(
        input: Bitmap,
        order: Int = 6,
        lineWidth: Float = 0.85f,
        contrast: Float = 2.5f,
        ink: Int = 0xFF18232B.toInt(),
        paper: Int = 0xFFF5F0E4.toInt(),
        strength: Float = 1f
    ): Bitmap

    /** Build a connected maze whose routes and wall widths follow image luminance. */
    suspend fun mazePortrait(
        input: Bitmap,
        cellSize: Float = 12f,
        wallWidth: Float = 0.46f,
        bias: Float = 3f,
        seed: Int = 1,
        ink: Int = 0xFF18232B.toInt(),
        paper: Int = 0xFFF5F0E4.toInt(),
        strength: Float = 1f
    ): Bitmap

    /** Turn image colors into overlapping yarn loops with knitted relief and fiber twist. */
    suspend fun knittedFabric(
        input: Bitmap,
        stitchSize: Float = 16f,
        yarnWidth: Float = 0.2f,
        relief: Float = 0.7f,
        twist: Float = 0.6f,
        strength: Float = 1f
    ): Bitmap

    /** Fold the image into a continuous triangular paper relief with lit faces and creases. */
    suspend fun origamiRelief(
        input: Bitmap,
        cellSize: Float = 72f,
        foldHeight: Float = 0.5f,
        lightAngle: Float = 315f,
        crease: Float = 0.45f,
        strength: Float = 1f
    ): Bitmap

    /** Curl the lower-right corner around a cylinder, revealing the back of the page and a transparent opening. */
    suspend fun pageCurl(
        input: Bitmap,
        amount: Float = 0.25f,
        radius: Float = 0.08f,
        angle: Float = 35f,
        shadow: Float = 0.6f,
        backside: Int = 0xFFFFF8E7.toInt(),
        strength: Float = 1f
    ): Bitmap

    /** Transport the image through a divergence-free noise field to create swirling fluid motion. */
    suspend fun curlAdvection(
        input: Bitmap,
        scale: Float = 4f,
        distance: Float = 48f,
        steps: Int = 16,
        seed: Int = 1,
        strength: Float = 1f
    ): Bitmap

    /** Replace each image cell with a tiny luminance histogram colored by the pixels in each bin. */
    suspend fun histogramMosaic(
        input: Bitmap,
        cells: Int = 16,
        bins: Int = 8,
        gap: Float = 0.08f,
        strength: Float = 1f
    ): Bitmap

    /** Encode eight circular neighbor comparisons as a grayscale texture descriptor. */
    suspend fun localBinaryPattern(
        input: Bitmap,
        radius: Float = 3f,
        tolerance: Float = 0.02f,
        strength: Float = 1f
    ): Bitmap

    /** Measure local tonal uncertainty. Flat regions become dark and varied texture becomes bright. */
    suspend fun localEntropyMap(
        input: Bitmap,
        radius: Float = 12f,
        bins: Int = 16,
        gain: Float = 1f,
        strength: Float = 1f
    ): Bitmap

    /** Rebuild the image with one continuous thread joining circularly arranged nails, choosing each chord to reduce the remaining tonal error. */
    suspend fun nailStringArt(
        input: Bitmap,
        nails: Int = 128,
        threads: Int = 1200,
        threadWidth: Float = 0.8f,
        contrast: Float = 1f,
        ink: Int = 0xFF18232B.toInt(),
        paper: Int = 0xFFF5F0E4.toInt(),
        strength: Float = 1f
    ): Bitmap

    /** Fold the fore-edges of closely spaced book pages into an image-guided relief with visible paper edges and shadowed gaps. */
    suspend fun foldedBook(
        input: Bitmap,
        pages: Int = 96,
        depth: Float = 48f,
        viewAngle: Float = 25f,
        paper: Int = 0xFFF4E8CD.toInt(),
        strength: Float = 1f
    ): Bitmap

    /** Reconstruct the image with upright spiral paper strips, colored edges and relief shadows. */
    suspend fun paperQuilling(
        input: Bitmap,
        cellSize: Float = 40f,
        turns: Float = 3.5f,
        stripWidth: Float = 0.06f,
        relief: Float = 0.8f,
        background: Int = 0xFFF5F0E4.toInt(),
        strength: Float = 1f
    ): Bitmap

    /** Refract the image through a sheet of cylindrical lenses, with viewing angle and lens rotation controlling the visible distortion. */
    suspend fun lenticularPrint(
        input: Bitmap,
        lensWidth: Float = 16f,
        viewAngle: Float = 0f,
        refraction: Float = 1.5f,
        gloss: Float = 0.45f,
        rotation: Float = 0f,
        strength: Float = 1f
    ): Bitmap

    /** Build a three-dimensional pin board from image luminance, with shaded pin tops, sides and overlaps. */
    suspend fun pinArt(
        input: Bitmap,
        spacing: Float = 18f,
        height: Float = 0.065f,
        angle: Float = 35f,
        gloss: Float = 0.65f,
        strength: Float = 1f
    ): Bitmap

    /** Place the image under glossy ceramic glaze with pigment pooling, fine crackle and relief highlights. */
    suspend fun ceramicGlaze(
        input: Bitmap,
        size: Float = 32f,
        pooling: Float = 0.55f,
        gloss: Float = 0.7f,
        crackle: Float = 0.5f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    /** Construct a wood inlay image with individually oriented grain and beveled seams between pieces. */
    suspend fun marquetry(
        input: Bitmap,
        size: Float = 42f,
        grain: Float = 0.7f,
        bevel: Float = 0.65f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap

    /** Visualize an image-derived stress tensor through crossed polarizers as colored interference fringes. */
    suspend fun photoelasticStress(
        input: Bitmap,
        load: Float = 2.4f,
        smoothing: Float = 8f,
        angle: Float = 25f,
        ambient: Float = 0.22f,
        strength: Float = 1f
    ): Bitmap

    /** View the image through a layer of refracting condensation droplets with variable coverage and a soft glass haze. */
    suspend fun condensation(
        input: Bitmap,
        size: Float = 48f,
        coverage: Float = 0.65f,
        refraction: Float = 0.7f,
        haze: Float = 0.18f,
        seed: Int = 0,
        strength: Float = 1f
    ): Bitmap
}
