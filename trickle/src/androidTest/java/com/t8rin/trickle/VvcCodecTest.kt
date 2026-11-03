package com.t8rin.trickle

import android.graphics.Bitmap
import android.graphics.Color
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import kotlinx.coroutines.runBlocking

@RunWith(AndroidJUnit4::class)
class VvcCodecTest {

    @Test
    fun opaqueOddSizedHeifCanBeDecodedSampled() = runBlocking {
        val source = Bitmap.createBitmap(593, 791, Bitmap.Config.ARGB_8888).apply {
            eraseColor(Color.rgb(32, 96, 192))
        }

        val encoded = VvcEncoder.encode(source)
        val decoded = VvcDecoder.decodeSampled(
            encoded = encoded,
            scaledWidth = 200,
            scaledHeight = 200,
            scaleMode = VvcScaleMode.FIT
        )

        assertEquals(150, decoded.width)
        assertEquals(200, decoded.height)
        assertTrue(VvcDecoder.isSupported(encoded))
    }

    @Test
    fun transparentOddSizedHeifPreservesAlpha() = runBlocking {
        val source = Bitmap.createBitmap(9, 11, Bitmap.Config.ARGB_8888).apply {
            eraseColor(Color.argb(127, 32, 96, 192))
        }

        val encoded = VvcEncoder.encode(
            bitmap = source,
            options = VvcEncoder.Options(lossless = true)
        )
        val decoded = VvcDecoder.decode(encoded)

        assertEquals(9, decoded.width)
        assertEquals(11, decoded.height)
        assertEquals(127, Color.alpha(decoded.getPixel(4, 5)))
    }

    @Test
    fun opaqueRawVvcWorksWithDefaultAlphaOption() = runBlocking {
        val source = Bitmap.createBitmap(8, 8, Bitmap.Config.ARGB_8888).apply {
            eraseColor(Color.rgb(32, 96, 192))
        }

        val encoded = VvcEncoder.encode(
            bitmap = source,
            options = VvcEncoder.Options(
                lossless = true,
                container = VvcContainer.RAW_VVC
            )
        )
        val decoded = VvcDecoder.decode(encoded)

        assertEquals(8, decoded.width)
        assertEquals(8, decoded.height)
    }
}
