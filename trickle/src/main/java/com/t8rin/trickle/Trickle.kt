package com.t8rin.trickle

import com.t8rin.trickle.pipeline.DitheringPipelineImpl
import com.t8rin.trickle.pipeline.EffectsPipelineImpl
import com.t8rin.trickle.pipeline.LowPolyPipelineImpl
import com.t8rin.trickle.pipeline.NativeLoader
import com.t8rin.trickle.pipeline.ResizePipelineImpl

object Trickle : NativeLoader(),
    LowPolyPipeline by LowPolyPipelineImpl,
    EffectsPipeline by EffectsPipelineImpl,
    DitheringPipeline by DitheringPipelineImpl,
    ResizePipeline by ResizePipelineImpl
