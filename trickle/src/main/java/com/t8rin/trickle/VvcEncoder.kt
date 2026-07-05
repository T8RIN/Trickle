@file:Suppress("KotlinJniMissingFunction")

package com.t8rin.trickle

import android.graphics.Bitmap
import androidx.annotation.IntRange
import com.t8rin.trickle.TrickleUtils.safe

object VvcEncoder {

    init {
        System.loadLibrary("vvc_jni")
    }

    fun encode(
        bitmap: Bitmap,
        options: Options = Options(),
    ): ByteArray {
        require(
            !options.preserveAlpha || (
                    options.orientation == VvcOrientation.NORMAL &&
                            options.contentLightLevel == null &&
                            options.exif == null
                    )
        ) {
            "VVC encoder cannot currently combine alpha with orientation, CLLI, or Exif metadata"
        }
        val cicp = options.cicp
        val contentLightLevel = options.contentLightLevel
        return encodeNative(
            bitmap = bitmap.safe(),
            quality = options.quality,
            lossless = options.lossless,
            chroma = options.chroma.value,
            bitDepth = options.bitDepth.value,
            threads = options.threads,
            rdoq = options.rdoq,
            aq = options.aq,
            mtt = options.mtt,
            lfnst = options.lfnst,
            depQuant = options.depQuant,
            mts = options.mts,
            dualTree = options.dualTree,
            cclm = options.cclm,
            deblock = options.deblock,
            container = options.container.value,
            preserveAlpha = options.preserveAlpha,
            hasCicp = cicp != null,
            primaries = cicp?.primaries?.value ?: 0,
            transfer = cicp?.transfer?.value ?: 0,
            matrix = cicp?.matrix?.value ?: 0,
            fullRange = cicp?.fullRange ?: false,
            iccProfile = options.iccProfile ?: byteArrayOf(),
            orientation = options.orientation.exifValue,
            maxCll = contentLightLevel?.maxContentLightLevel ?: -1,
            maxFall = contentLightLevel?.maxPictureAverageLightLevel ?: -1,
            exif = options.exif ?: byteArrayOf()
        ) ?: error("VVC: native encoder returned null")
    }

    data class Options(
        @param:IntRange(from = 1, to = 100)
        val quality: Int = 90,
        val lossless: Boolean = false,
        val chroma: VvcChroma = VvcChroma.YUV_420,
        val bitDepth: VvcBitDepth = VvcBitDepth.EIGHT,
        @param:IntRange(from = 0)
        val threads: Int = 1,
        val rdoq: Boolean = true,
        val aq: Boolean = true,
        val mtt: Boolean = true,
        val lfnst: Boolean = true,
        val depQuant: Boolean = true,
        val mts: Boolean = true,
        val dualTree: Boolean = true,
        val cclm: Boolean = true,
        val deblock: Boolean = true,
        val container: VvcContainer = VvcContainer.HEIF,
        val preserveAlpha: Boolean = false,
        val cicp: VvcCicp? = VvcCicp.DEFAULT,
        val iccProfile: ByteArray? = null,
        val orientation: VvcOrientation = VvcOrientation.NORMAL,
        val contentLightLevel: VvcContentLightLevel? = null,
        val exif: ByteArray? = null,
    ) {
        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (javaClass != other?.javaClass) return false

            other as Options

            if (quality != other.quality) return false
            if (lossless != other.lossless) return false
            if (threads != other.threads) return false
            if (rdoq != other.rdoq) return false
            if (aq != other.aq) return false
            if (mtt != other.mtt) return false
            if (lfnst != other.lfnst) return false
            if (depQuant != other.depQuant) return false
            if (mts != other.mts) return false
            if (dualTree != other.dualTree) return false
            if (cclm != other.cclm) return false
            if (deblock != other.deblock) return false
            if (preserveAlpha != other.preserveAlpha) return false
            if (chroma != other.chroma) return false
            if (bitDepth != other.bitDepth) return false
            if (container != other.container) return false
            if (cicp != other.cicp) return false
            if (!iccProfile.contentEquals(other.iccProfile)) return false
            if (orientation != other.orientation) return false
            if (contentLightLevel != other.contentLightLevel) return false
            if (!exif.contentEquals(other.exif)) return false

            return true
        }

        override fun hashCode(): Int {
            var result = quality
            result = 31 * result + lossless.hashCode()
            result = 31 * result + threads
            result = 31 * result + rdoq.hashCode()
            result = 31 * result + aq.hashCode()
            result = 31 * result + mtt.hashCode()
            result = 31 * result + lfnst.hashCode()
            result = 31 * result + depQuant.hashCode()
            result = 31 * result + mts.hashCode()
            result = 31 * result + dualTree.hashCode()
            result = 31 * result + cclm.hashCode()
            result = 31 * result + deblock.hashCode()
            result = 31 * result + preserveAlpha.hashCode()
            result = 31 * result + chroma.hashCode()
            result = 31 * result + bitDepth.hashCode()
            result = 31 * result + container.hashCode()
            result = 31 * result + (cicp?.hashCode() ?: 0)
            result = 31 * result + (iccProfile?.contentHashCode() ?: 0)
            result = 31 * result + orientation.hashCode()
            result = 31 * result + (contentLightLevel?.hashCode() ?: 0)
            result = 31 * result + (exif?.contentHashCode() ?: 0)
            return result
        }
    }

    private external fun encodeNative(
        bitmap: Bitmap,
        quality: Int,
        lossless: Boolean,
        chroma: Int,
        bitDepth: Int,
        threads: Int,
        rdoq: Boolean,
        aq: Boolean,
        mtt: Boolean,
        lfnst: Boolean,
        depQuant: Boolean,
        mts: Boolean,
        dualTree: Boolean,
        cclm: Boolean,
        deblock: Boolean,
        container: Int,
        preserveAlpha: Boolean,
        hasCicp: Boolean,
        primaries: Int,
        transfer: Int,
        matrix: Int,
        fullRange: Boolean,
        iccProfile: ByteArray,
        orientation: Int,
        maxCll: Int,
        maxFall: Int,
        exif: ByteArray,
    ): ByteArray?
}
