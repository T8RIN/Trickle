package com.t8rin.trickle

data class NtscSettings(
    val randomSeed: Int = 0,
    val useField: UseField = UseField.INTERLEAVED_UPPER,
    val filterType: FilterType = FilterType.BUTTERWORTH,
    val inputLumaFilter: LumaLowpass = LumaLowpass.NOTCH,
    val chromaLowpassIn: ChromaLowpass = ChromaLowpass.FULL,
    val chromaDemodulation: ChromaDemodulationFilter = ChromaDemodulationFilter.NOTCH,
    val lumaSmear: Float = 0.5f,
    val compositeSharpening: Float = 1f,
    val videoScanlinePhaseShift: PhaseShift = PhaseShift.DEGREES_180,
    val videoScanlinePhaseShiftOffset: Int = 0,
    val headSwitching: HeadSwitching? = HeadSwitching(),
    val trackingNoise: TrackingNoise? = TrackingNoise(),
    val compositeNoise: FbmNoise? = FbmNoise(
        frequency = 0.5f,
        intensity = 0.05f,
        detail = 1
    ),
    val ringing: Ringing? = Ringing(),
    val lumaNoise: FbmNoise? = FbmNoise(
        frequency = 0.5f,
        intensity = 0.01f,
        detail = 1
    ),
    val chromaNoise: FbmNoise? = FbmNoise(
        frequency = 0.05f,
        intensity = 0.1f,
        detail = 2
    ),
    val snowIntensity: Float = 0.00025f,
    val snowAnisotropy: Float = 0.5f,
    val chromaPhaseNoiseIntensity: Float = 0.001f,
    val chromaPhaseError: Float = 0f,
    val chromaDelayHorizontal: Float = 0f,
    val chromaDelayVertical: Int = 0,
    val vhs: VHS? = VHS(),
    val chromaVertBlend: Boolean = true,
    val chromaLowpassOut: ChromaLowpass = ChromaLowpass.FULL,
    val scale: Scale? = Scale(),
) {
    companion object {
        val DEFAULT = NtscSettings()
    }

    enum class UseField {
        ALTERNATING,
        UPPER,
        LOWER,
        BOTH,
        INTERLEAVED_UPPER,
        INTERLEAVED_LOWER,
    }

    enum class FilterType {
        CONSTANT_K,
        BUTTERWORTH,
    }

    enum class LumaLowpass {
        NONE,
        BOX,
        NOTCH,
    }

    enum class PhaseShift {
        DEGREES_0,
        DEGREES_90,
        DEGREES_180,
        DEGREES_270,
    }

    enum class VHSTapeSpeed {
        NONE,
        SP,
        LP,
        EP,
    }

    enum class ChromaLowpass {
        NONE,
        LIGHT,
        FULL,
    }

    enum class ChromaDemodulationFilter {
        BOX,
        NOTCH,
        ONE_LINE_COMB,
        TWO_LINE_COMB,
    }

    data class HeadSwitching(
        val height: Int = 8,
        val offset: Int = 3,
        val horizontalShift: Float = 72f,
        val midLine: HeadSwitchingMidLine? = HeadSwitchingMidLine(),
    )

    data class HeadSwitchingMidLine(
        val position: Float = 0.95f,
        val jitter: Float = 0.03f,
    )

    data class TrackingNoise(
        val height: Int = 12,
        val waveIntensity: Float = 15f,
        val snowIntensity: Float = 0.025f,
        val snowAnisotropy: Float = 0.25f,
        val noiseIntensity: Float = 0.25f,
    )

    data class Ringing(
        val frequency: Float = 0.45f,
        val power: Float = 4f,
        val intensity: Float = 4f,
    )

    data class FbmNoise(
        val frequency: Float = 0f,
        val intensity: Float = 0f,
        val detail: Int = 0,
    )

    data class VHS(
        val tapeSpeed: VHSTapeSpeed = VHSTapeSpeed.LP,
        val chromaLoss: Float = 0.000025f,
        val sharpen: VHSSharpen? = VHSSharpen(),
        val edgeWave: VHSEdgeWave? = VHSEdgeWave(),
    )

    data class VHSSharpen(
        val intensity: Float = 0.25f,
        val frequency: Float = 1f,
    )

    data class VHSEdgeWave(
        val intensity: Float = 0.5f,
        val speed: Float = 4f,
        val frequency: Float = 0.05f,
        val detail: Int = 2,
    )

    data class Scale(
        val horizontal: Float = 1f,
        val vertical: Float = 1f,
        val scaleWithVideoSize: Boolean = false,
    )
}
