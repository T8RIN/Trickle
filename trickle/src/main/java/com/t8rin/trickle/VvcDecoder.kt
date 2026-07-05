@file:Suppress("KotlinJniMissingFunction")

package com.t8rin.trickle

import android.graphics.Bitmap
import android.graphics.Matrix
import java.nio.ByteBuffer
import java.nio.ByteOrder

object VvcDecoder {

    init {
        System.loadLibrary("vvc_jni")
    }

    fun decode(
        encoded: ByteArray,
        options: Options = Options(),
    ): Result {
        val container = options.container ?: if (isRawVvc(encoded)) {
            VvcContainer.RAW_VVC
        } else {
            VvcContainer.HEIF
        }
        val packet = decodeNative(encoded, container.value)
            ?: error("VVC: native decoder returned null")
        return parsePacket(packet, options.applyOrientation)
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

    data class Result(
        val bitmap: Bitmap,
        val width: Int,
        val height: Int,
        val chroma: VvcChroma,
        val bitDepth: VvcBitDepth,
        val orientation: VvcOrientation,
        val hasAlpha: Boolean,
        val cicp: VvcCicp?,
        val iccProfile: ByteArray?,
    )

    private fun parsePacket(packet: ByteArray, applyOrientation: Boolean): Result {
        require(packet.size >= HEADER_SIZE && packet.copyOfRange(0, 4).contentEquals(MAGIC)) {
            "Invalid VVC native response"
        }
        val buffer = ByteBuffer.wrap(packet).order(ByteOrder.LITTLE_ENDIAN)
        buffer.position(4)
        val width = buffer.int
        val height = buffer.int
        val chromaValue = buffer.get().toInt()
        val bitDepthValue = buffer.get().toInt()
        val orientationValue = buffer.get().toInt()
        val chroma = VvcChroma.entries.first { it.value == chromaValue }
        val bitDepth = VvcBitDepth.entries.first { it.value == bitDepthValue }
        val orientation = VvcOrientation.entries.first { it.exifValue == orientationValue }
        val hasAlpha = buffer.get().toInt() != 0
        val primaries = buffer.short.toInt() and 0xFFFF
        val transfer = buffer.short.toInt() and 0xFFFF
        val matrix = buffer.short.toInt() and 0xFFFF
        val fullRange = buffer.get().toInt() != 0
        val iccLength = buffer.int
        val rgbaLength = buffer.int
        require(width > 0 && height > 0 && iccLength >= 0 && rgbaLength == width * height * 4) {
            "Invalid VVC decoded image dimensions"
        }
        require(buffer.remaining() == iccLength + rgbaLength) {
            "Invalid VVC decoded image payload"
        }
        val icc = ByteArray(iccLength).also(buffer::get).takeIf { it.isNotEmpty() }
        val rgba = ByteArray(rgbaLength).also(buffer::get)
        val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888).also {
            it.copyPixelsFromBuffer(ByteBuffer.wrap(rgba))
        }
        val orientedBitmap = if (applyOrientation) bitmap.applyOrientation(orientation) else bitmap
        val cicp = if (primaries == NO_CICP) null else VvcCicp(
            primaries = VvcCicp.Primaries.entries.firstOrNull { it.value == primaries }
                ?: VvcCicp.Primaries.UNSPECIFIED,
            transfer = VvcCicp.Transfer.entries.firstOrNull { it.value == transfer }
                ?: VvcCicp.Transfer.UNSPECIFIED,
            matrix = VvcCicp.Matrix.entries.firstOrNull { it.value == matrix }
                ?: VvcCicp.Matrix.UNSPECIFIED,
            fullRange = fullRange
        )
        return Result(
            bitmap = orientedBitmap,
            width = width,
            height = height,
            chroma = chroma,
            bitDepth = bitDepth,
            orientation = orientation,
            hasAlpha = hasAlpha,
            cicp = cicp,
            iccProfile = icc
        )
    }

    private fun Bitmap.applyOrientation(orientation: VvcOrientation): Bitmap {
        if (orientation == VvcOrientation.NORMAL) return this
        val matrix = Matrix().apply {
            when (orientation) {
                VvcOrientation.NORMAL -> Unit
                VvcOrientation.FLIP_H -> postScale(-1f, 1f)
                VvcOrientation.ROTATE_180 -> postRotate(180f)
                VvcOrientation.FLIP_V -> postScale(1f, -1f)
                VvcOrientation.TRANSPOSE -> {
                    postScale(-1f, 1f)
                    postRotate(90f)
                }

                VvcOrientation.ROTATE_90 -> postRotate(90f)
                VvcOrientation.TRANSVERSE -> {
                    postScale(-1f, 1f)
                    postRotate(-90f)
                }

                VvcOrientation.ROTATE_270 -> postRotate(-90f)
            }
        }
        return Bitmap.createBitmap(this, 0, 0, width, height, matrix, true)
    }

    private fun isRawVvc(encoded: ByteArray): Boolean {
        return encoded.size >= 3 && encoded[0] == 0.toByte() && encoded[1] == 0.toByte() && (
                encoded[2] == 1.toByte() ||
                        encoded.size >= 4 && encoded[2] == 0.toByte() && encoded[3] == 1.toByte()
                )
    }

    private external fun decodeNative(encoded: ByteArray, container: Int): ByteArray?

    private const val HEADER_SIZE = 31
    private const val NO_CICP = 0xFFFF
    val MAGIC = "VVC1".encodeToByteArray()
}
