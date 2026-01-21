package com.t8rin.trickle.pipeline

abstract class NativeLoader {
    init {
        System.loadLibrary("trickle")
    }
}