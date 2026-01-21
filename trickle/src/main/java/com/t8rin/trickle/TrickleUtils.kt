package com.t8rin.trickle

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Color
import android.net.Uri
import com.t8rin.trickle.pipeline.NativeLoader
import java.io.File

object TrickleUtils : NativeLoader() {

    fun generateShades(color: Int, shadeStep: Int = 5, from: Int = 2, to: Int = 98): List<Int> {
        val shades = mutableListOf<Int>()

        val alpha = Color.alpha(color)

        val hsv = FloatArray(3)
        Color.colorToHSV(color, hsv)

        for (i in from..to step shadeStep) {
            shades.add(
                Color.HSVToColor(
                    alpha,
                    shade(hsv, i)
                )
            )
        }

        return shades
    }

    fun shade(hsv: FloatArray, tone: Int): FloatArray {
        val valueFactor = tone / 100.0f
        val shadedHsv = hsv.copyOf()
        shadedHsv[2] = valueFactor

        return shadedHsv
    }

    fun getAbsolutePath(
        uri: Uri,
        context: Context,
        filename: String = uri.toString().takeLastWhile { it != '/' }
    ): String {
        val file = File(
            context.cacheDir,
            filename
        )
        context.contentResolver.openInputStream(uri)!!.use {
            it.copyTo(file.outputStream())
        }
        return file.absolutePath
    }

    fun Bitmap.checkHasAlpha(): Boolean = checkHasAlphaImpl(this)

    fun calculateBrightness(
        bitmap: Bitmap,
        pixelSpacing: Int = 1
    ) = calculateBrightnessEstimate(
        bitmap = bitmap.safe(),
        pixelSpacing = pixelSpacing
    )

    fun fastBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap = fastBlurImpl(
        srcBitmap = bitmap.safe(),
        scale = scale,
        radius = radius
    )

    fun stackBlur(
        bitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap = stackBlurImpl(
        srcBitmap = bitmap.safe(),
        scale = scale,
        radius = radius
    )

    fun trimEmptyParts(
        bitmap: Bitmap,
        transparent: Int
    ): Bitmap = trimEmptyPartsImpl(
        bitmap = bitmap.safe(),
        transparent = transparent
    )

    private external fun trimEmptyPartsImpl(
        bitmap: Bitmap,
        transparent: Int
    ): Bitmap

    private external fun stackBlurImpl(
        srcBitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap

    private external fun fastBlurImpl(
        srcBitmap: Bitmap,
        scale: Float,
        radius: Int
    ): Bitmap

    private external fun checkHasAlphaImpl(bitmap: Bitmap): Boolean

    private external fun calculateBrightnessEstimate(
        bitmap: Bitmap,
        pixelSpacing: Int = 1
    ): Int

    private fun Bitmap.safe() = if (config != Bitmap.Config.ARGB_8888) {
        copy(Bitmap.Config.ARGB_8888, false)
    } else this

}