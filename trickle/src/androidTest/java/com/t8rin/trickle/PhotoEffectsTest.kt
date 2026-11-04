package com.t8rin.trickle

import android.graphics.Bitmap
import android.graphics.Color
import androidx.test.ext.junit.runners.AndroidJUnit4
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.asCoroutineDispatcher
import kotlinx.coroutines.cancelAndJoin
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withContext
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertSame
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.Executors
import kotlin.math.abs
import kotlin.system.measureTimeMillis

@RunWith(AndroidJUnit4::class)
class PhotoEffectsTest {

    @Test
    fun effectsAreDeterministicAndDoNotMutateTheirInput() = runBlocking {
        val input = image(129, 97)
        val before = pixels(input)
        try {
            for ((name, effect) in effects) {
                assertSame(name, input, effect(input, 0f))
                val first = effect(input, 1f)
                val second = effect(input, 1f)
                try {
                    assertEquals(name, input.width, first.width)
                    assertEquals(name, input.height, first.height)
                    assertEquals(name, input.density, first.density)
                    assertArrayEquals(name, pixels(first), pixels(second))
                    assertArrayEquals(name, before, pixels(input))
                    assertFalse("$name produced no effect", before.contentEquals(pixels(first)))
                } finally {
                    if (first !== input) first.recycle()
                    if (second !== input && second !== first) second.recycle()
                }
            }
        } finally {
            input.recycle()
        }
    }

    @Test
    fun tinyNarrowAndRgb565InputsAreSupported() = runBlocking {
        for ((width, height) in listOf(1 to 1, 1 to 17, 19 to 1, 7 to 9)) {
            for (config in listOf(Bitmap.Config.ARGB_8888, Bitmap.Config.RGB_565)) {
                val input = image(width, height).let { original ->
                    if (config == original.config) original else original.copy(config, false).also {
                        original.recycle()
                    }
                }
                try {
                    for ((name, effect) in effects) {
                        val result = effect(input, 1f)
                        assertEquals(name, width, result.width)
                        assertEquals(name, height, result.height)
                        if (result !== input) result.recycle()
                    }
                } finally {
                    input.recycle()
                }
            }
        }
    }

    @Test
    fun condensationDoesNotDarkenColorsAlongTransparentEdges() = runBlocking {
        for ((width, height) in listOf(64 to 64, 1025 to 31)) {
            val colors = IntArray(width * height) { index ->
                if (index % width < width / 2) {
                    Color.argb(if (index / width % 2 == 0) 255 else 128, 255, 0, 0)
                } else Color.TRANSPARENT
            }
            val input = Bitmap.createBitmap(colors, width, height, Bitmap.Config.ARGB_8888)
            val reference = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888).apply {
                eraseColor(Color.RED)
            }
            try {
                for (droplets in listOf(false, true)) {
                    suspend fun render(bitmap: Bitmap) = Trickle.condensation(
                        input = bitmap,
                        size = 80f,
                        coverage = if (droplets) 1f else 0f,
                        refraction = 1.5f,
                        haze = if (droplets) 0f else 0.6f,
                        seed = 123
                    )
                    val actual = render(input)
                    val expected = render(reference)
                    try {
                        val actualPixels = pixels(actual)
                        val expectedPixels = pixels(expected)
                        for (index in colors.indices) {
                            val alpha = Color.alpha(colors[index])
                            assertEquals("Alpha changed at $index", alpha, Color.alpha(actualPixels[index]))
                            if (alpha == 0) continue
                            for (shift in listOf(0, 8, 16)) {
                                val actualChannel = actualPixels[index] shr shift and 255
                                val expectedChannel = expectedPixels[index] shr shift and 255
                                assertTrue(
                                    "Transparent background changed color at $index, droplets=$droplets",
                                    abs(actualChannel - expectedChannel) <= 2
                                )
                            }
                        }
                    } finally {
                        if (actual !== input) actual.recycle()
                        if (expected !== reference) expected.recycle()
                    }
                }
            } finally {
                input.recycle()
                reference.recycle()
            }
        }
    }

    @Test
    fun pageCurlAmountControlsTheFoldIndependentlyOfStrength() = runBlocking {
        val input = image(129, 97)
        val before = pixels(input).apply {
            for (index in indices) this[index] = this[index] or Color.BLACK
        }
        input.setPixels(before, 0, input.width, 0, 0, input.width, input.height)
        val results = mutableListOf<Bitmap>()
        try {
            val flat = Trickle.pageCurl(input, amount = 0f).also(results::add)
            assertArrayEquals("Zero curl must preserve the image", before, pixels(flat))
            val default = Trickle.pageCurl(input).also(results::add)
            val defaultPixels = pixels(default)
            assertFalse("Default curl produced no effect", before.contentEquals(defaultPixels))
            assertTrue(
                "Default curl removed most of the page",
                defaultPixels.count { Color.alpha(it) > 0 } > before.size / 2
            )
            val small = Trickle.pageCurl(input, amount = 0.15f).also(results::add)
            val large = Trickle.pageCurl(input, amount = 0.5f).also(results::add)
            assertFalse("Curl amount was ignored", pixels(small).contentEquals(pixels(large)))
        } finally {
            results.distinct().filter { it !== input }.forEach(Bitmap::recycle)
            input.recycle()
        }
    }

    @Test
    fun cancellationStopsNativeWorkAndDoesNotPoisonTheNextOperation() = runBlocking {
        Executors.newSingleThreadExecutor().asCoroutineDispatcher().use { dispatcher ->
            val input = image(2048, 1536)
            try {
                val workloads: List<Pair<String, suspend () -> Bitmap>> = listOf(
                    "FlowTexture" to { Trickle.flowTexture(input, length = 0.1f) },
                    "NailStringArt" to { Trickle.nailStringArt(input, nails = 256, threads = 3000) }
                )
                for ((name, workload) in workloads) {
                    val entered = CompletableDeferred<Unit>()
                    val job = launch(dispatcher) {
                        entered.complete(Unit)
                        val result = workload()
                        if (result !== input) result.recycle()
                    }
                    entered.await()
                    delay(20)
                    assertTrue("$name finished before cancellation could be checked", job.isActive)
                    val elapsed = measureTimeMillis { job.cancelAndJoin() }
                    assertTrue("$name cancellation took $elapsed ms", elapsed < 3000)
                }
                withContext(dispatcher) {
                    val tiny = image(9, 7)
                    try {
                        val result = Trickle.thinFilmIridescence(tiny)
                        assertEquals(9, result.width)
                        if (result !== tiny) result.recycle()
                    } finally {
                        tiny.recycle()
                    }
                }
            } finally {
                input.recycle()
            }
        }
    }

    private fun image(width: Int, height: Int): Bitmap =
        Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888).apply {
            density = 240
            val colors = IntArray(width * height) { index ->
                val x = index % width
                val y = index / width
                Color.argb((x * 23 + y * 11) and 255,
                    (x * 17 + y * 5) and 255,
                    (x * 3 + y * 11) and 255,
                    (x * 7 + y * 13) and 255)
            }
            setPixels(colors, 0, width, 0, 0, width, height)
        }

    private fun pixels(bitmap: Bitmap): IntArray = IntArray(bitmap.width * bitmap.height).also {
        bitmap.getPixels(it, 0, bitmap.width, 0, 0, bitmap.width, bitmap.height)
    }

    private val effects: List<Pair<String, suspend (Bitmap, Float) -> Bitmap>> = listOf(
        "HeightAmbientOcclusion" to { input, strength -> Trickle.heightAmbientOcclusion(input, strength = strength) },
        "HeightCastShadows" to { input, strength -> Trickle.heightCastShadows(input, strength = strength) },
        "RefractiveCaustics" to { input, strength -> Trickle.refractiveCaustics(input, strength = strength) },
        "ThinFilmIridescence" to { input, strength -> Trickle.thinFilmIridescence(input, strength = strength) },
        "DiffractionGrating" to { input, strength -> Trickle.diffractionGrating(input, strength = strength) },
        "FlowTexture" to { input, strength -> Trickle.flowTexture(input, strength = strength) },
        "HilbertRibbon" to { input, strength -> Trickle.hilbertRibbon(input, strength = strength) },
        "MazePortrait" to { input, strength -> Trickle.mazePortrait(input, strength = strength) },
        "KnittedFabric" to { input, strength -> Trickle.knittedFabric(input, strength = strength) },
        "OrigamiRelief" to { input, strength -> Trickle.origamiRelief(input, strength = strength) },
        "PageCurl" to { input, strength -> Trickle.pageCurl(input, strength = strength) },
        "CurlAdvection" to { input, strength -> Trickle.curlAdvection(input, strength = strength) },
        "HistogramMosaic" to { input, strength -> Trickle.histogramMosaic(input, strength = strength) },
        "LocalBinaryPattern" to { input, strength -> Trickle.localBinaryPattern(input, strength = strength) },
        "LocalEntropyMap" to { input, strength -> Trickle.localEntropyMap(input, strength = strength) },
        "NailStringArt" to { input, strength -> Trickle.nailStringArt(input, strength = strength) },
        "FoldedBook" to { input, strength -> Trickle.foldedBook(input, strength = strength) },
        "PaperQuilling" to { input, strength -> Trickle.paperQuilling(input, strength = strength) },
        "LenticularPrint" to { input, strength -> Trickle.lenticularPrint(input, strength = strength) },
        "PinArt" to { input, strength -> Trickle.pinArt(input, strength = strength) },
        "CeramicGlaze" to { input, strength -> Trickle.ceramicGlaze(input, strength = strength) },
        "Marquetry" to { input, strength -> Trickle.marquetry(input, strength = strength) },
        "PhotoelasticStress" to { input, strength -> Trickle.photoelasticStress(input, strength = strength) },
        "Condensation" to { input, strength -> Trickle.condensation(input, strength = strength) }
    )
}
