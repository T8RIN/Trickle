package com.t8rin.trickle

import com.t8rin.trickle.pipeline.DitheringPipelineImpl
import com.t8rin.trickle.pipeline.EffectsPipelineImpl
import com.t8rin.trickle.pipeline.LowPolyPipelineImpl

object Trickle : LowPolyPipeline by LowPolyPipelineImpl,
    EffectsPipeline by EffectsPipelineImpl,
    DitheringPipeline by DitheringPipelineImpl {

    init {
        System.loadLibrary("trickle")
    }

}