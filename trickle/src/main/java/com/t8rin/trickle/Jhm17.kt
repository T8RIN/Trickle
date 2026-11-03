@file:Suppress("KotlinJniMissingFunction", "unused")

package com.t8rin.trickle

import android.graphics.Bitmap
import androidx.annotation.FloatRange
import androidx.annotation.IntRange
import com.t8rin.trickle.Jhm17.Options.Additional
import com.t8rin.trickle.TrickleUtils.safe

object Jhm17 {

    init {
        System.loadLibrary("jhm17_jni")
    }

    /** Encodes a bitmap as JPEG, discarding alpha. */
    fun encode(
        bitmap: Bitmap,
        options: Options = Options(),
    ): ByteArray {
        require(options.quality in 1..100) { "JPEG quality must be in 1..100" }
        require(!bitmap.isRecycled) { "Bitmap has been recycled" }
        require(bitmap.width in 1..65535 && bitmap.height in 1..65535) {
            "JPEG dimensions must be in 1..65535"
        }
        val additional = options.additional
        val grayscale = additional?.grayscale == true
        if (options.polish != Polish.OFF || additional?.polishOptions != null) {
            require(
                !grayscale && options.chroma in listOf(
                    Chroma.AUTO,
                    Chroma.YUV_420,
                    Chroma.YUV_444
                )
            ) {
                "JPEG polish requires color output with 4:2:0 or 4:4:4 chroma subsampling"
            }
        }
        val appSegments = additional?.appSegments?.toList()
        return encodeNative(
            bitmap = bitmap.safe(),
            width = bitmap.width,
            height = bitmap.height,
            quality = options.quality,
            progressive = options.progressive,
            chroma = options.chroma.value,
            polish = options.polish.value,
            grayscale = grayscale,
            settings = additional?.nativeSettings(),
            rdo = additional?.rdo?.nativeValues(),
            lumaTable = additional?.lumaTable?.nativeValues(),
            chromaTable = additional?.chromaTable?.nativeValues(),
            lumaStepTuning = additional?.lumaStepTuning?.nativeValues(),
            cbStepTuning = additional?.cbStepTuning?.nativeValues(),
            crStepTuning = additional?.crStepTuning?.nativeValues(),
            deadzone = additional?.deadzone?.nativeValues(),
            polishOptions = additional?.polishOptions?.nativeValues(),
            iccProfile = additional?.iccProfile,
            exif = additional?.exif,
            appNumbers = appSegments?.map { it.number }?.toIntArray(),
            appData = appSegments?.map { it.data }?.toTypedArray()
        ) ?: error("jhm17: native encoder returned null")
    }

    /**
     * @property polish Refines JPEG coefficients for 4:2:0 and 4:4:4.
     * Uses a lot of memory and is off by default.
     */
    data class Options(
        @param:IntRange(from = 1, to = 100)
        val quality: Int = 90,
        val progressive: Boolean = false,
        val chroma: Chroma = Chroma.AUTO,
        val polish: Polish = Polish.OFF,
        val additional: Additional? = null,
    ) {
        /**
         * @property progressiveMode Overrides [Options.progressive] when supplied.
         * @property progressiveScans Number of scans per component in [ProgressiveMode.SPECTRAL].
         * @property optimizedHuffmanTables Controls optional optimization. jhm17 also optimizes
         * tables for progressive scans, RDO and polish where required.
         * @property arithmeticCoding Requires a decoder with arithmetic JPEG support.
         * @property restartInterval Number of MCUs between restart markers. Zero disables them.
         * @property iccProfile Profile for the encoded samples. Color bitmaps are read as sRGB.
         * @property exif Raw TIFF payload. jhm17 adds the Exif header.
         * @property polishOptions Enables custom polish and overrides [Options.polish].
         * Uses a lot of memory.
         */
        data class Additional(
            val progressiveMode: ProgressiveMode? = null,
            @param:IntRange(from = 2, to = 64)
            val progressiveScans: Int = 5,
            val optimizedHuffmanTables: Boolean? = null,
            val arithmeticCoding: Boolean? = null,
            @param:IntRange(from = 0, to = 65535)
            val restartInterval: Int? = null,
            val grayscale: Boolean = false,
            val chromaSubsamplingMethod: ChromaSubsamplingMethod? = null,
            val samplePrecision: SamplePrecision? = null,
            val autoChromaScale: Boolean? = null,
            val density: PixelDensity? = null,
            val iccProfile: ByteArray? = null,
            val exif: ByteArray? = null,
            val appSegments: List<AppSegment>? = null,
            val rdo: Rdo? = null,
            val lumaTable: QuantizationTable? = null,
            val chromaTable: QuantizationTable? = null,
            val lumaStepTuning: StepTuning? = null,
            val cbStepTuning: StepTuning? = null,
            val crStepTuning: StepTuning? = null,
            val deadzone: Deadzone? = null,
            val polishOptions: PolishOptions? = null,
        ) {
            enum class ProgressiveMode(internal val value: Int) {
                SEQUENTIAL(0),
                OPTIMIZED(1),
                STANDARD(2),
                SPECTRAL(3),
            }

            enum class ChromaSubsamplingMethod(internal val value: Int) {
                NEAREST(0),
                AVERAGE(1),
            }

            enum class SamplePrecision(internal val value: Int) {
                INTEGER(0),
                LUMA(1),
                ALL(2),
            }

            data class PixelDensity(
                @param:IntRange(from = 1, to = 65535)
                val x: Int,
                @param:IntRange(from = 1, to = 65535)
                val y: Int = x,
                val unit: Unit = Unit.INCHES,
            ) {
                enum class Unit(internal val value: Int) {
                    PIXEL_ASPECT_RATIO(0),
                    INCHES(1),
                    CENTIMETERS(2),
                }
            }

            data class AppSegment(
                @param:IntRange(from = 1, to = 15)
                val number: Int,
                val data: ByteArray,
            ) {
                override fun equals(other: Any?): Boolean {
                    if (this === other) return true
                    if (javaClass != other?.javaClass) return false

                    other as AppSegment

                    if (number != other.number) return false
                    if (!data.contentEquals(other.data)) return false

                    return true
                }

                override fun hashCode(): Int {
                    var result = number
                    result = 31 * result + data.contentHashCode()
                    return result
                }
            }

            /** Set [lambda] to zero to disable rate-distortion optimization. */
            data class Rdo(
                val lambda: Float = 0.05f,
                val chromaWeight: Float = 3f,
                val chromaWeightFull: Float = 1f,
                val distortionPower: Float = 0.35f,
                @param:FloatRange(from = 0.0, to = 1.0)
                val tileWeights: Float = 0.5f,
                val tileChroma: Boolean = true,
                val tileChromaFull: Boolean = false,
                val aqStrength: Float = 0.75f,
                val aqMin: Float = 0.5f,
                val aqMax: Float = 2f,
                val aqChroma: Boolean = true,
                val aqBrightness: Float = 0.5f,
                val aqBrightnessMax: Float = 8f,
                @param:IntRange(from = 0, to = 255)
                val clipIterations: Int = 3,
            )

            sealed interface QuantizationTable {
                enum class Preset(internal val value: Int) : QuantizationTable {
                    ANNEX_K(0),
                    FLAT(1),
                    MS_SSIM(2),
                    PSNR_HVS(3),
                    IMAGE_MAGICK(4),
                    KLEIN_SILVERSTEIN_CARNEY(5),
                    DENTAL_X_RAYS(6),
                    VISUAL_DETECTION_MODEL(7),
                    IMPROVED_DETECTION_MODEL(8),
                }

                /** 64 quantization steps in row-major DCT order, each in 1..255. */
                data class Custom(val steps: List<Int>) : QuantizationTable
            }

            /** Multipliers for DC, AC and the five AC frequency bands. */
            data class StepTuning(
                val dc: Float = 0.5f,
                val ac: Float = 1f,
                val bands: List<Float> = List(5) { 1f },
            )

            /** Thresholds for the four frequency quadrants. Used when RDO is disabled. */
            data class Deadzone(
                val luma: List<Float> = listOf(0.58f, 0.635f, 0.66f, 0.7f),
                val chroma: List<Float> = List(4) { 0.5f },
                val chromaFull: List<Float> = List(4) { 0.65f },
            )

            data class PolishOptions(
                val judge: Judge = Judge.BOTH,
                val proposals: Int = 4,
                val rounds: Int = 2,
                val maxBits: Int = 2,
                val restoreThreshold: Float = 0.25f,
                @param:IntRange(from = 0, to = 63)
                val scanLimit: Int = 63,
                val cvvdpTries: Int = 2,
                @param:IntRange(from = 1, to = 99)
                val qualityStep: Int = 3,
                @param:IntRange(from = 0, to = 4)
                val trellisK: Int = 0,
                val trellisProposals: Int = 6,
            ) {
                enum class Judge(internal val value: Int) {
                    SSIMULACRA_2(0),
                    CVVDP(1),
                    BOTH(2),
                }
            }

            override fun equals(other: Any?): Boolean {
                if (this === other) return true
                if (javaClass != other?.javaClass) return false

                other as Additional

                if (progressiveScans != other.progressiveScans) return false
                if (optimizedHuffmanTables != other.optimizedHuffmanTables) return false
                if (arithmeticCoding != other.arithmeticCoding) return false
                if (restartInterval != other.restartInterval) return false
                if (grayscale != other.grayscale) return false
                if (autoChromaScale != other.autoChromaScale) return false
                if (progressiveMode != other.progressiveMode) return false
                if (chromaSubsamplingMethod != other.chromaSubsamplingMethod) return false
                if (samplePrecision != other.samplePrecision) return false
                if (density != other.density) return false
                if (!iccProfile.contentEquals(other.iccProfile)) return false
                if (!exif.contentEquals(other.exif)) return false
                if (appSegments != other.appSegments) return false
                if (rdo != other.rdo) return false
                if (lumaTable != other.lumaTable) return false
                if (chromaTable != other.chromaTable) return false
                if (lumaStepTuning != other.lumaStepTuning) return false
                if (cbStepTuning != other.cbStepTuning) return false
                if (crStepTuning != other.crStepTuning) return false
                if (deadzone != other.deadzone) return false
                if (polishOptions != other.polishOptions) return false

                return true
            }

            override fun hashCode(): Int {
                var result = progressiveScans
                result = 31 * result + optimizedHuffmanTables.hashCode()
                result = 31 * result + arithmeticCoding.hashCode()
                result = 31 * result + restartInterval.hashCode()
                result = 31 * result + grayscale.hashCode()
                result = 31 * result + autoChromaScale.hashCode()
                result = 31 * result + progressiveMode.hashCode()
                result = 31 * result + chromaSubsamplingMethod.hashCode()
                result = 31 * result + samplePrecision.hashCode()
                result = 31 * result + density.hashCode()
                result = 31 * result + (iccProfile?.contentHashCode() ?: 0)
                result = 31 * result + (exif?.contentHashCode() ?: 0)
                result = 31 * result + appSegments.hashCode()
                result = 31 * result + rdo.hashCode()
                result = 31 * result + lumaTable.hashCode()
                result = 31 * result + chromaTable.hashCode()
                result = 31 * result + lumaStepTuning.hashCode()
                result = 31 * result + cbStepTuning.hashCode()
                result = 31 * result + crStepTuning.hashCode()
                result = 31 * result + deadzone.hashCode()
                result = 31 * result + polishOptions.hashCode()
                return result
            }
        }
    }

    enum class Chroma(internal val value: Int) {
        AUTO(0),
        YUV_420(1),
        YUV_422(2),
        YUV_444(3),
        YUV_440(4),
        YUV_441(5),
        YUV_421(6),
        YUV_411(7),
        YUV_410(8),
    }

    enum class Polish(internal val value: Int) {
        OFF(0),
        GREEDY(1),
        TRELLIS(2),
        TRELLIS_GREEDY(3),
    }

    private fun Additional.nativeSettings(): IntArray {
        require(progressiveMode != Additional.ProgressiveMode.SPECTRAL || progressiveScans in 2..64) {
            "Spectral JPEG requires 2..64 scans per component"
        }
        require(restartInterval == null || restartInterval in 0..65535) {
            "JPEG restart interval must be in 0..65535"
        }
        require(density == null || density.x in 1..65535 && density.y in 1..65535) {
            "JPEG pixel density must be in 1..65535"
        }
        return intArrayOf(
            progressiveMode?.value ?: -1,
            progressiveScans,
            optimizedHuffmanTables.nativeValue(),
            arithmeticCoding.nativeValue(),
            restartInterval ?: -1,
            chromaSubsamplingMethod?.value ?: -1,
            samplePrecision?.value ?: -1,
            autoChromaScale.nativeValue(),
            density?.unit?.value ?: -1,
            density?.x ?: 0,
            density?.y ?: 0
        )
    }

    private fun Boolean?.nativeValue(): Int = when (this) {
        true -> 1
        false -> 0
        null -> -1
    }

    private fun Additional.Rdo.nativeValues(): FloatArray {
        val values = floatArrayOf(
            lambda, chromaWeight, chromaWeightFull, distortionPower, tileWeights,
            tileChroma.nativeValue().toFloat(), tileChromaFull.nativeValue().toFloat(),
            aqStrength, aqMin, aqMax, aqChroma.nativeValue().toFloat(),
            aqBrightness, aqBrightnessMax, clipIterations.toFloat()
        )
        require(values.all { it.isFinite() } && lambda >= 0f && chromaWeight > 0f &&
                chromaWeightFull > 0f && tileWeights in 0f..1f && aqMin > 0f &&
                aqMax >= aqMin && aqBrightnessMax > 0f && clipIterations in 0..255) {
            "Invalid JPEG RDO options"
        }
        return values
    }

    private fun Additional.QuantizationTable.nativeValues(): IntArray = when (this) {
        is Additional.QuantizationTable.Preset -> intArrayOf(value)
        is Additional.QuantizationTable.Custom -> steps.toIntArray().also { values ->
            require(values.size == 64 && values.all { it in 1..255 }) {
                "JPEG quantization tables require 64 steps in 1..255"
            }
        }
    }

    private fun Additional.StepTuning.nativeValues(): FloatArray {
        val values = floatArrayOf(dc, ac) + bands.toFloatArray()
        require(values.size == 7 && values.all { it.isFinite() && it > 0f }) {
            "JPEG step tuning requires positive DC, AC and five band multipliers"
        }
        return values
    }

    private fun Additional.Deadzone.nativeValues(): FloatArray {
        val lumaValues = luma.toFloatArray()
        val chromaValues = chroma.toFloatArray()
        val fullValues = chromaFull.toFloatArray()
        require(lumaValues.size == 4 && chromaValues.size == 4 && fullValues.size == 4) {
            "JPEG deadzone requires four thresholds per component group"
        }
        val values = lumaValues + chromaValues + fullValues
        require(values.all { it.isFinite() && it >= 0f }) {
            "JPEG deadzone thresholds must be finite and non-negative"
        }
        return values
    }

    private fun Additional.PolishOptions.nativeValues(): IntArray {
        require(
            proposals >= 0 && rounds >= 0 && restoreThreshold.isFinite() &&
                    scanLimit in 0..63 && cvvdpTries >= 0 && qualityStep in 1..99 &&
                    trellisK in 0..4 && trellisProposals >= 0
        ) {
            "Invalid JPEG polish options"
        }
        return intArrayOf(
            judge.value, proposals, rounds, maxBits, restoreThreshold.toRawBits(),
            scanLimit, cvvdpTries, qualityStep, trellisK, trellisProposals
        )
    }

    private external fun encodeNative(
        bitmap: Bitmap,
        width: Int,
        height: Int,
        quality: Int,
        progressive: Boolean,
        chroma: Int,
        polish: Int,
        grayscale: Boolean,
        settings: IntArray?,
        rdo: FloatArray?,
        lumaTable: IntArray?,
        chromaTable: IntArray?,
        lumaStepTuning: FloatArray?,
        cbStepTuning: FloatArray?,
        crStepTuning: FloatArray?,
        deadzone: FloatArray?,
        polishOptions: IntArray?,
        iccProfile: ByteArray?,
        exif: ByteArray?,
        appNumbers: IntArray?,
        appData: Array<ByteArray>?,
    ): ByteArray?
}
