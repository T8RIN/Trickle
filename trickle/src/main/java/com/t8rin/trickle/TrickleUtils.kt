package com.t8rin.trickle

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Color
import android.net.Uri
import java.io.File

object TrickleUtils {

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

    fun getAbsolutePath(uri: Uri, context: Context): String {
        val file = File(context.filesDir, "lut.${uri.toString().takeLastWhile { it != '.' }}")
        context.contentResolver.openInputStream(uri)!!.use {
            it.copyTo(file.outputStream())
        }
        return file.absolutePath
    }

    fun Bitmap.checkHasAlpha(): Boolean = checkHasAlphaImpl(this)

    private external fun checkHasAlphaImpl(bitmap: Bitmap): Boolean

}