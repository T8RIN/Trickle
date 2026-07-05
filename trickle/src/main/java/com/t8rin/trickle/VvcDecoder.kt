@file:Suppress("KotlinJniMissingFunction")

package com.t8rin.trickle

import android.graphics.Bitmap
import androidx.annotation.IntRange

object VvcDecoder {

    init {
        System.loadLibrary("vvc_jni")
    }

    fun decode(
        encoded: ByteArray,
        options: Options = Options(),
    ): Bitmap {
        return decodeNative(
            encoded = encoded,
            container = options.resolveContainer(encoded).value,
            scaledWidth = 0,
            scaledHeight = 0,
            scaleMode = VvcScaleMode.FIT.value,
            applyOrientation = options.applyOrientation
        ) ?: error("VVC: native decoder returned null")
    }

    fun decodeSampled(
        encoded: ByteArray,
        @IntRange(from = 1) scaledWidth: Int,
        @IntRange(from = 1) scaledHeight: Int,
        scaleMode: VvcScaleMode = VvcScaleMode.FIT,
        options: Options = Options(),
    ): Bitmap {
        require(scaledWidth > 0 && scaledHeight > 0) {
            "Scaled dimensions must be positive"
        }
        return decodeNative(
            encoded = encoded,
            container = options.resolveContainer(encoded).value,
            scaledWidth = scaledWidth,
            scaledHeight = scaledHeight,
            scaleMode = scaleMode.value,
            applyOrientation = options.applyOrientation
        ) ?: error("VVC: native decoder returned null")
    }

    fun isSupported(encoded: ByteArray): Boolean {
        val isHeif = encoded.size >= 12 && encoded.copyOfRange(4, 8)
            .contentEquals("ftyp".encodeToByteArray())
        return isRawVvc(encoded) || isHeif
    }

    data class Options(
        val container: VvcContainer? = null,
        val applyOrientation: Boolean = true,
    )

    private fun Options.resolveContainer(encoded: ByteArray): VvcContainer {
        return container ?: if (isRawVvc(encoded)) {
            VvcContainer.RAW_VVC
        } else {
            VvcContainer.HEIF
        }
    }

    private fun isRawVvc(encoded: ByteArray): Boolean {
        return encoded.size >= 3 && encoded[0] == 0.toByte() && encoded[1] == 0.toByte() && (
                encoded[2] == 1.toByte() ||
                        encoded.size >= 4 && encoded[2] == 0.toByte() && encoded[3] == 1.toByte()
                )
    }

    private external fun decodeNative(
        encoded: ByteArray,
        container: Int,
        scaledWidth: Int,
        scaledHeight: Int,
        scaleMode: Int,
        applyOrientation: Boolean,
    ): Bitmap?
}
