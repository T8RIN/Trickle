package com.t8rin.trickle

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Color
import androidx.test.ext.junit.runners.AndroidJUnit4
import com.t8rin.trickle.Jhm17.Options.Additional
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import kotlin.math.abs
import kotlinx.coroutines.runBlocking

@RunWith(AndroidJUnit4::class)
class Jhm17Test {

    @Test
    fun jpegPreservesStraightColorsOfTranslucentBitmap() = runBlocking {
        val source = Bitmap.createBitmap(9, 11, Bitmap.Config.ARGB_8888).apply {
            eraseColor(Color.argb(128, 32, 96, 192))
        }

        val encoded = Jhm17.encode(source, Jhm17.Options(quality = 100))
        val decoded = BitmapFactory.decodeByteArray(encoded, 0, encoded.size)
        val pixel = decoded.getPixel(4, 5)

        assertEquals(9, decoded.width)
        assertEquals(11, decoded.height)
        assertEquals(255, Color.alpha(pixel))
        assertTrue(abs(Color.red(pixel) - 32) <= 8)
        assertTrue(abs(Color.green(pixel) - 96) <= 8)
        assertTrue(abs(Color.blue(pixel) - 192) <= 8)
    }

    @Test
    fun progressiveJpegWithChromaSubsamplingCanBeDecoded() = runBlocking {
        val source = Bitmap.createBitmap(17, 19, Bitmap.Config.RGB_565).apply {
            eraseColor(Color.BLUE)
        }

        val encoded = Jhm17.encode(
            bitmap = source,
            options = Jhm17.Options(progressive = true, chroma = Jhm17.Chroma.YUV_420)
        )
        val decoded = BitmapFactory.decodeByteArray(encoded, 0, encoded.size)

        assertTrue((0 until encoded.lastIndex).any {
            encoded[it] == 0xff.toByte() && encoded[it + 1] == 0xc2.toByte()
        })
        assertEquals(17, decoded.width)
        assertEquals(19, decoded.height)
    }

    @Test
    fun polishModesChangeJpegAndKeepItDecodable() = runBlocking {
        val width = 64
        val height = 48
        val pixels = IntArray(width * height) { index ->
            val x = index % width
            val y = index / width
            val ripple = (x * 13 + y * 7) % 17 * 5
            Color.rgb((x * 4) xor ripple, y * 5, x * y % 200 + ripple / 2)
        }
        val source = Bitmap.createBitmap(pixels, width, height, Bitmap.Config.ARGB_8888)
        val options = Jhm17.Options(quality = 60)

        val baseline = Jhm17.encode(source, options)
        val modes = listOf(Jhm17.Polish.GREEDY, Jhm17.Polish.TRELLIS, Jhm17.Polish.TRELLIS_GREEDY)
        for (mode in modes) {
            val polished = Jhm17.encode(source, options.copy(polish = mode))
            val decoded = BitmapFactory.decodeByteArray(polished, 0, polished.size)

            assertFalse("$mode left the JPEG unchanged", polished.contentEquals(baseline))
            assertEquals(width, decoded.width)
            assertEquals(height, decoded.height)
        }

        val greedy = Jhm17.encode(source, options.copy(polish = Jhm17.Polish.GREEDY))
        val custom = Jhm17.encode(
            source,
            options.copy(additional = Additional(polishOptions = Additional.PolishOptions()))
        )
        assertArrayEquals(greedy, custom)
    }

    @Test
    fun emptyAdditionalUsesEncoderDefaults() = runBlocking {
        val source = Bitmap.createBitmap(19, 21, Bitmap.Config.ARGB_8888).apply {
            eraseColor(Color.GREEN)
        }
        assertArrayEquals(
            Jhm17.encode(source),
            Jhm17.encode(source, Jhm17.Options(additional = Additional()))
        )
    }

    @Test
    fun advancedProgressiveSettingsAreWrittenToJpeg() = runBlocking {
        val source = Bitmap.createBitmap(25, 19, Bitmap.Config.ARGB_8888).apply {
            eraseColor(Color.RED)
        }
        val additional = Additional(
            grayscale = true,
            progressiveMode = Additional.ProgressiveMode.SPECTRAL,
            progressiveScans = 4,
            restartInterval = 1,
            density = Additional.PixelDensity(x = 300, y = 200),
            lumaTable = Additional.QuantizationTable.Custom(List(64) { 20 }),
            appSegments = listOf(Additional.AppSegment(12, byteArrayOf(5, 6))),
            exif = byteArrayOf(0x49, 0x49, 0x2a, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        )
        val encoded = Jhm17.encode(source, Jhm17.Options(additional = additional))
        val decoded = BitmapFactory.decodeByteArray(encoded, 0, encoded.size)
        val pixel = decoded.getPixel(0, 0)

        assertEquals(25, decoded.width)
        assertEquals(19, decoded.height)
        assertEquals(Color.red(pixel), Color.green(pixel))
        assertEquals(Color.red(pixel), Color.blue(pixel))
        assertEquals(1, encoded.headerSegment(0xc2)[5].toInt())
        assertEquals(4, (0 until encoded.lastIndex).count {
            encoded[it] == 0xff.toByte() && encoded[it + 1] == 0xda.toByte()
        })
        assertArrayEquals(byteArrayOf(0, 1), encoded.headerSegment(0xdd))
        assertArrayEquals(byteArrayOf(1, 1, 44, 0, 200.toByte()), encoded.headerSegment(0xe0).copyOfRange(7, 12))
        assertArrayEquals(ByteArray(64) { 20 }, encoded.headerSegment(0xdb).copyOfRange(1, 65))
        assertArrayEquals(byteArrayOf(5, 6), encoded.headerSegment(0xec))
        assertArrayEquals("Exif\u0000\u0000".encodeToByteArray() + additional.exif!!, encoded.headerSegment(0xe1))
    }

    @Test
    fun arithmeticModeOverridesProgressiveFlag() = runBlocking {
        val source = Bitmap.createBitmap(17, 17, Bitmap.Config.ARGB_8888).apply {
            eraseColor(Color.BLUE)
        }
        val icc = byteArrayOf(10, 20, 30)
        val encoded = Jhm17.encode(
            source,
            Jhm17.Options(
                progressive = true,
                chroma = Jhm17.Chroma.YUV_411,
                additional = Additional(
                    progressiveMode = Additional.ProgressiveMode.SEQUENTIAL,
                    arithmeticCoding = true,
                    samplePrecision = Additional.SamplePrecision.INTEGER,
                    chromaSubsamplingMethod = Additional.ChromaSubsamplingMethod.NEAREST,
                    autoChromaScale = false,
                    rdo = Additional.Rdo(lambda = 0f),
                    deadzone = Additional.Deadzone(),
                    cbStepTuning = Additional.StepTuning(),
                    crStepTuning = Additional.StepTuning(ac = 1.2f),
                    chromaTable = Additional.QuantizationTable.Preset.ANNEX_K,
                    iccProfile = icc
                )
            )
        )

        assertEquals(0x41, encoded.headerSegment(0xc9)[7].toInt())
        assertArrayEquals(
            "ICC_PROFILE\u0000".encodeToByteArray() + byteArrayOf(1, 1) + icc,
            encoded.headerSegment(0xe2)
        )
    }

    private fun ByteArray.headerSegment(marker: Int): ByteArray {
        var offset = 2
        while (offset + 3 < size && this[offset] == 0xff.toByte()) {
            val type = this[offset + 1].toInt() and 0xff
            val length = ((this[offset + 2].toInt() and 0xff) shl 8) or
                    (this[offset + 3].toInt() and 0xff)
            if (type == marker) return copyOfRange(offset + 4, offset + 2 + length)
            if (type == 0xda) break
            offset += 2 + length
        }
        error("JPEG marker ${marker.toString(16)} was not found")
    }
}
