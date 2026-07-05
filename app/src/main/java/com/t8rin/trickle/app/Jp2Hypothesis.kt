package com.t8rin.trickle.app

import android.graphics.Bitmap
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.PickVisualMediaRequest
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Slider
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asAndroidBitmap
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.core.graphics.drawable.toBitmap
import androidx.core.graphics.scale
import coil.compose.AsyncImage
import coil.imageLoader
import coil.request.ImageRequest
import coil.size.Size
import coil.transform.Transformation
import coil.util.DebugLogger
import com.t8rin.trickle.NtscSettings
import com.t8rin.trickle.Trickle
import com.t8rin.trickle.VvcDecoder
import com.t8rin.trickle.VvcEncoder
import com.t8rin.trickle.WarpBrush
import com.t8rin.trickle.WarpEngine
import com.t8rin.trickle.WarpMode
import kotlin.math.roundToInt
import kotlin.random.Random


@Composable
fun MainActivity.Jp2Hypothesis() {
    var source by remember {
        mutableStateOf("")
    }

    var target by remember {
        mutableStateOf("")
    }


    val imagePicker =
        rememberLauncherForActivityResult(contract = ActivityResultContracts.PickVisualMedia()) {
            source = it?.toString() ?: ""
        }

    val pickImage: () -> Unit = {
        imagePicker.launch(PickVisualMediaRequest())
    }

    rememberLauncherForActivityResult(contract = ActivityResultContracts.PickVisualMedia()) {
        target = it?.toString() ?: ""
    }

    val docuemntPicker =
        rememberLauncherForActivityResult(contract = ActivityResultContracts.OpenDocument()) {
            target = it?.toString() ?: ""
        }

    val pickImage2: () -> Unit = {
        docuemntPicker.launch(arrayOf("*/*"))
    }

    var intensity by remember {
        mutableStateOf(1f)
    }
    var colorValue by remember {
        mutableStateOf(1f)
    }
    var ntscFrame by remember {
        mutableStateOf(0)
    }
    var ntscScaleFactorX by remember {
        mutableStateOf(1f)
    }
    var ntscScaleFactorY by remember {
        mutableStateOf(1f)
    }
    var ntscSettings by remember {
        mutableStateOf(NtscSettings.DEFAULT)
    }

    Scaffold(
        topBar = {
//            Text("NTSC")
//            AsyncImage(
//                model = remember(
//                    source,
//                    ntscSettings,
//                    ntscFrame,
//                    ntscScaleFactorX,
//                    ntscScaleFactorY
//                ) {
//                    ImageRequest.Builder(this@Jp2Hypothesis).allowHardware(false)
//                        .data(source)
//                        .transformations(
//                            GenericTransformation(
//                                key = listOf(
//                                    ntscSettings,
//                                    ntscFrame,
//                                    ntscScaleFactorX,
//                                    ntscScaleFactorY
//                                )
//                            ) { bmp ->
//                                Trickle.ntsc(
//                                    src = bmp,
//                                    frame = ntscFrame,
//                                    scaleFactorX = ntscScaleFactorX,
//                                    scaleFactorY = ntscScaleFactorY,
//                                    settings = ntscSettings
//                                )
//                            }
//                        )
//                        .build()
//                },
//                imageLoader = imageLoader,
//                modifier = Modifier.height(280.dp),
//                contentDescription = null,
//                contentScale = ContentScale.Fit
//            )
        }
    ) { contentPadding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(24.dp)
                .verticalScroll(rememberScrollState())
                .padding(contentPadding)
        ) {
            val imageLoader = remember {
                imageLoader.newBuilder().logger(DebugLogger()).build()
            }
            var isGray by remember {
                mutableStateOf(true)
            }
            Row(
                modifier = Modifier.height(300.dp)
            ) {
                AsyncImage(
                    model = remember(source, imageLoader) {
                        ImageRequest.Builder(this@Jp2Hypothesis)
                            .data(source)
                            .allowHardware(false)
                            .transformations(
                                listOf(
                                    object : Transformation {
                                        override val cacheKey: String
                                            get() = Random.nextInt().toString()

                                        override suspend fun transform(
                                            input: Bitmap,
                                            size: Size
                                        ): Bitmap = VvcDecoder.decode(
                                            VvcEncoder.encode(
                                                input.scale(593, 791),
                                                VvcEncoder.Options()
                                            ),
                                            VvcDecoder.Options()
                                        )
                                    }
                                )
                            )
                            .build()
                    },
                    imageLoader = imageLoader,
                    modifier = Modifier.weight(1f),
                    contentDescription = null
                )
                AsyncImage(
                    model = remember(target, imageLoader) {
                        ImageRequest.Builder(this@Jp2Hypothesis)
                            .data(target)
                            .allowHardware(false)
                            .transformations(
                                listOf(
                                    object : Transformation {
                                        override val cacheKey: String
                                            get() = Random.nextInt().toString()

                                        override suspend fun transform(
                                            input: Bitmap,
                                            size: Size
                                        ): Bitmap = Trickle.drawColorAbove(
                                            input,
                                            Color.Red.copy(0.8f).toArgb()
                                        )
                                    }
                                )
                            )
                            .build()
                    },
                    imageLoader = imageLoader,
                    modifier = Modifier.weight(1f),
                    contentDescription = null
                )
                Switch(checked = isGray, onCheckedChange = { isGray = it })
            }


//            NtscControls(
//                frame = ntscFrame,
//                onFrameChange = { ntscFrame = it },
//                scaleFactorX = ntscScaleFactorX,
//                onScaleFactorXChange = { ntscScaleFactorX = it },
//                scaleFactorY = ntscScaleFactorY,
//                onScaleFactorYChange = { ntscScaleFactorY = it },
//                settings = ntscSettings,
//                onSettingsChange = { ntscSettings = it }
//            )

            Row(
                modifier = Modifier.height(360.dp)
            ) {
//            Column(
//                modifier = Modifier
//                    .weight(1f)
//                    .verticalScroll(rememberScrollState())
//            ) {
//                AsyncImage(
//                    model = remember(source, target, intensity, colorValue, colors) {
//                        ImageRequest.Builder(this@Jp2Hypothesis).allowHardware(false)
//                            .data(source)
//                            .transformations(
//                                if (intensity == 1f) {
//                                    listOf(
//                                        GenericTransformation { bmp ->
//                                            List(4) { it }.fold(bmp) { acc, i ->
//                                                when (i % 5) {
//                                                    1 -> Trickle.shuffleBlur(acc, 128f, 0.2f)
//                                                    else -> acc
//                                                }
//                                            }
//                                        }
//                                    )
//                                } else emptyList()
//                            ).build()
//                    },
//                    imageLoader = imageLoader,
//                    modifier = Modifier.height(300.dp),
//                    contentDescription = null
//                )
////                DitheringType.entries.forEach { type ->
////                    Text(type.name)
////                    AsyncImage(
////                        model = remember(source, isGray, colors) {
////                            ImageRequest.Builder(this@Jp2Hypothesis).allowHardware(false)
////                                .data(source)
////                                .transformations(
////                                    GenericTransformation { bmp ->
////                                        Trickle.dithering(
////                                            input = bmp,
////                                            type = type,
////                                            threshold = 128,
////                                            isGrayScale = isGray
////                                        )
////                                    }
////                                ).build()
////                        },
////                        imageLoader = imageLoader,
////                        modifier = Modifier.height(300.dp),
////                        contentDescription = null
////                    )
////                }
//            }
                var last by remember { mutableStateOf<Offset?>(null) }
                var warpEngine by remember {
                    mutableStateOf<WarpEngine?>(null)
                }
                var invalidate by remember {
                    mutableStateOf(0)
                }
                val model by remember(invalidate) {
                    derivedStateOf {
                        warpEngine?.render()?.asImageBitmap() ?: ImageBitmap(1, 1)
                    }
                }
                LaunchedEffect(source) {
                    warpEngine = imageLoader.execute(
                        ImageRequest.Builder(this@Jp2Hypothesis).allowHardware(false)
                            .data(source)
                            .size(1200)
                            .build()
                    ).drawable?.toBitmap()?.let { WarpEngine(it) }
                }

                Image(
                    bitmap = model,
                    modifier = Modifier
                        .weight(1f)
                        .pointerInput(Unit) {
                            detectDragGestures(
                                onDragEnd = { last = null }
                            ) { change, _ ->
                                val engine = warpEngine ?: return@detectDragGestures
                                val p = change.position

                                last?.let {
                                    val from = mapToBitmap(it, size, model.asAndroidBitmap())
                                    val to = mapToBitmap(p, size, model.asAndroidBitmap())

                                    engine.applyStroke(
                                        from.x, from.y,
                                        to.x, to.y,
                                        WarpBrush(80f, 1f, 0.4f),
                                        WarpMode.MIXING
                                    )
                                    invalidate++
                                }
                                last = p
                            }
                        },
                    contentDescription = null,
                    contentScale = ContentScale.FillHeight
                )
            }

            Row {
                Button(onClick = pickImage) {
                    Text("Source")
                }
                Button(onClick = pickImage2) {
                    Text("Target")
                }
                Slider(value = intensity, onValueChange = { intensity = it }, valueRange = 0f..1f)
            }
            Slider(value = colorValue, onValueChange = { colorValue = it }, valueRange = 0f..1f)
        }
    }
}

@Composable
private fun NtscControls(
    frame: Int,
    onFrameChange: (Int) -> Unit,
    scaleFactorX: Float,
    onScaleFactorXChange: (Float) -> Unit,
    scaleFactorY: Float,
    onScaleFactorYChange: (Float) -> Unit,
    settings: NtscSettings,
    onSettingsChange: (NtscSettings) -> Unit
) {
    Section("Frame") {
        IntStepper("Frame", frame, 1, onFrameChange)
        FloatSlider("Scale factor X", scaleFactorX, 0.125f..8f, onScaleFactorXChange)
        FloatSlider("Scale factor Y", scaleFactorY, 0.125f..8f, onScaleFactorYChange)
        IntStepper("Random seed", settings.randomSeed, 1) {
            onSettingsChange(settings.copy(randomSeed = it))
        }
    }

    Section("Signal") {
        EnumCycle("Use field", settings.useField, NtscSettings.UseField.entries) {
            onSettingsChange(settings.copy(useField = it))
        }
        EnumCycle("Filter type", settings.filterType, NtscSettings.FilterType.entries) {
            onSettingsChange(settings.copy(filterType = it))
        }
        EnumCycle("Input luma filter", settings.inputLumaFilter, NtscSettings.LumaLowpass.entries) {
            onSettingsChange(settings.copy(inputLumaFilter = it))
        }
        EnumCycle(
            "Chroma lowpass in",
            settings.chromaLowpassIn,
            NtscSettings.ChromaLowpass.entries
        ) {
            onSettingsChange(settings.copy(chromaLowpassIn = it))
        }
        EnumCycle(
            "Chroma demodulation",
            settings.chromaDemodulation,
            NtscSettings.ChromaDemodulationFilter.entries
        ) {
            onSettingsChange(settings.copy(chromaDemodulation = it))
        }
        EnumCycle(
            "Phase shift",
            settings.videoScanlinePhaseShift,
            NtscSettings.PhaseShift.entries
        ) {
            onSettingsChange(settings.copy(videoScanlinePhaseShift = it))
        }
        IntStepper("Phase shift offset", settings.videoScanlinePhaseShiftOffset, 1) {
            onSettingsChange(settings.copy(videoScanlinePhaseShiftOffset = it))
        }
        FloatSlider("Luma smear", settings.lumaSmear, 0f..5f) {
            onSettingsChange(settings.copy(lumaSmear = it))
        }
        FloatSlider("Composite sharpening", settings.compositeSharpening, -5f..5f) {
            onSettingsChange(settings.copy(compositeSharpening = it))
        }
        FloatSlider("Snow intensity", settings.snowIntensity, 0f..0.01f) {
            onSettingsChange(settings.copy(snowIntensity = it))
        }
        FloatSlider("Snow anisotropy", settings.snowAnisotropy, 0f..1f) {
            onSettingsChange(settings.copy(snowAnisotropy = it))
        }
        FloatSlider("Chroma phase noise", settings.chromaPhaseNoiseIntensity, 0f..0.05f) {
            onSettingsChange(settings.copy(chromaPhaseNoiseIntensity = it))
        }
        FloatSlider("Chroma phase error", settings.chromaPhaseError, 0f..1f) {
            onSettingsChange(settings.copy(chromaPhaseError = it))
        }
        FloatSlider("Chroma delay horizontal", settings.chromaDelayHorizontal, -40f..40f) {
            onSettingsChange(settings.copy(chromaDelayHorizontal = it))
        }
        IntSlider("Chroma delay vertical", settings.chromaDelayVertical, -40..40) {
            onSettingsChange(settings.copy(chromaDelayVertical = it))
        }
        ToggleSetting("Chroma vertical blend", settings.chromaVertBlend) {
            onSettingsChange(settings.copy(chromaVertBlend = it))
        }
        EnumCycle(
            "Chroma lowpass out",
            settings.chromaLowpassOut,
            NtscSettings.ChromaLowpass.entries
        ) {
            onSettingsChange(settings.copy(chromaLowpassOut = it))
        }
    }

    NullableBlock(
        label = "Head switching",
        value = settings.headSwitching,
        defaultValue = NtscSettings.HeadSwitching(),
        onValueChange = { onSettingsChange(settings.copy(headSwitching = it)) }
    ) { value, update ->
        IntSlider("Height", value.height, 0..80) { update(value.copy(height = it)) }
        IntSlider("Offset", value.offset, -80..80) { update(value.copy(offset = it)) }
        FloatSlider("Horizontal shift", value.horizontalShift, -200f..200f) {
            update(value.copy(horizontalShift = it))
        }
        NullableBlock(
            label = "Mid line",
            value = value.midLine,
            defaultValue = NtscSettings.HeadSwitchingMidLine(),
            onValueChange = { update(value.copy(midLine = it)) }
        ) { midLine, updateMidLine ->
            FloatSlider("Position", midLine.position, 0f..1f) {
                updateMidLine(midLine.copy(position = it))
            }
            FloatSlider("Jitter", midLine.jitter, 0f..0.2f) {
                updateMidLine(midLine.copy(jitter = it))
            }
        }
    }

    NullableBlock(
        label = "Tracking noise",
        value = settings.trackingNoise,
        defaultValue = NtscSettings.TrackingNoise(),
        onValueChange = { onSettingsChange(settings.copy(trackingNoise = it)) }
    ) { value, update ->
        IntSlider("Height", value.height, 0..100) { update(value.copy(height = it)) }
        FloatSlider("Wave intensity", value.waveIntensity, 0f..80f) {
            update(value.copy(waveIntensity = it))
        }
        FloatSlider("Snow intensity", value.snowIntensity, 0f..0.5f) {
            update(value.copy(snowIntensity = it))
        }
        FloatSlider("Snow anisotropy", value.snowAnisotropy, 0f..1f) {
            update(value.copy(snowAnisotropy = it))
        }
        FloatSlider("Noise intensity", value.noiseIntensity, 0f..2f) {
            update(value.copy(noiseIntensity = it))
        }
    }

    FbmBlock("Composite noise", settings.compositeNoise, NtscSettings.FbmNoise(0.5f, 0.05f, 1)) {
        onSettingsChange(settings.copy(compositeNoise = it))
    }
    FbmBlock("Luma noise", settings.lumaNoise, NtscSettings.FbmNoise(0.5f, 0.01f, 1)) {
        onSettingsChange(settings.copy(lumaNoise = it))
    }
    FbmBlock("Chroma noise", settings.chromaNoise, NtscSettings.FbmNoise(0.05f, 0.1f, 2)) {
        onSettingsChange(settings.copy(chromaNoise = it))
    }

    NullableBlock(
        label = "Ringing",
        value = settings.ringing,
        defaultValue = NtscSettings.Ringing(),
        onValueChange = { onSettingsChange(settings.copy(ringing = it)) }
    ) { value, update ->
        FloatSlider("Frequency", value.frequency, 0f..1f) { update(value.copy(frequency = it)) }
        FloatSlider("Power", value.power, 0f..10f) { update(value.copy(power = it)) }
        FloatSlider("Intensity", value.intensity, 0f..10f) { update(value.copy(intensity = it)) }
    }

    NullableBlock(
        label = "VHS",
        value = settings.vhs,
        defaultValue = NtscSettings.VHS(),
        onValueChange = { onSettingsChange(settings.copy(vhs = it)) }
    ) { value, update ->
        EnumCycle("Tape speed", value.tapeSpeed, NtscSettings.VHSTapeSpeed.entries) {
            update(value.copy(tapeSpeed = it))
        }
        FloatSlider("Chroma loss", value.chromaLoss, 0f..0.01f) {
            update(value.copy(chromaLoss = it))
        }
        NullableBlock(
            label = "Sharpen",
            value = value.sharpen,
            defaultValue = NtscSettings.VHSSharpen(),
            onValueChange = { update(value.copy(sharpen = it)) }
        ) { sharpen, updateSharpen ->
            FloatSlider("Intensity", sharpen.intensity, 0f..5f) {
                updateSharpen(sharpen.copy(intensity = it))
            }
            FloatSlider("Frequency", sharpen.frequency, 0.5f..4f) {
                updateSharpen(sharpen.copy(frequency = it))
            }
        }
        NullableBlock(
            label = "Edge wave",
            value = value.edgeWave,
            defaultValue = NtscSettings.VHSEdgeWave(),
            onValueChange = { update(value.copy(edgeWave = it)) }
        ) { edgeWave, updateEdgeWave ->
            FloatSlider("Intensity", edgeWave.intensity, 0f..20f) {
                updateEdgeWave(edgeWave.copy(intensity = it))
            }
            FloatSlider("Speed", edgeWave.speed, 0f..10f) {
                updateEdgeWave(edgeWave.copy(speed = it))
            }
            FloatSlider("Frequency", edgeWave.frequency, 0f..0.5f) {
                updateEdgeWave(edgeWave.copy(frequency = it))
            }
            IntSlider("Detail", edgeWave.detail, 1..5) {
                updateEdgeWave(edgeWave.copy(detail = it))
            }
        }
    }

    NullableBlock(
        label = "Scale settings",
        value = settings.scale,
        defaultValue = NtscSettings.Scale(),
        onValueChange = { onSettingsChange(settings.copy(scale = it)) }
    ) { value, update ->
        FloatSlider("Horizontal", value.horizontal, 0.125f..8f) {
            update(value.copy(horizontal = it))
        }
        FloatSlider("Vertical", value.vertical, 0.125f..8.8f) {
            update(value.copy(vertical = it))
        }
        ToggleSetting("Scale with video size", value.scaleWithVideoSize) {
            update(value.copy(scaleWithVideoSize = it))
        }
    }
}

@Composable
private fun Section(
    title: String,
    content: @Composable ColumnScope.() -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(top = 12.dp)
    ) {
        Text(title)
        content()
    }
}

@Composable
private fun <T : Enum<T>> EnumCycle(
    label: String,
    value: T,
    values: List<T>,
    onValueChange: (T) -> Unit
) {
    Button(
        onClick = {
            val next = values[(values.indexOf(value) + 1) % values.size]
            onValueChange(next)
        },
        modifier = Modifier.fillMaxWidth()
    ) {
        Text("$label: ${value.name}")
    }
}

@Composable
private fun FloatSlider(
    label: String,
    value: Float,
    range: ClosedFloatingPointRange<Float>,
    onValueChange: (Float) -> Unit
) {
    Column(modifier = Modifier.fillMaxWidth()) {
        Text("$label: ${value.shortString()}")
        Slider(
            value = value.coerceIn(range.start, range.endInclusive),
            onValueChange = onValueChange,
            valueRange = range,
            modifier = Modifier.fillMaxWidth()
        )
    }
}

@Composable
private fun IntSlider(
    label: String,
    value: Int,
    range: IntRange,
    onValueChange: (Int) -> Unit
) {
    Column(modifier = Modifier.fillMaxWidth()) {
        Text("$label: $value")
        Slider(
            value = value.coerceIn(range.first, range.last).toFloat(),
            onValueChange = { onValueChange(it.roundToInt().coerceIn(range.first, range.last)) },
            valueRange = range.first.toFloat()..range.last.toFloat(),
            steps = (range.last - range.first - 1).coerceAtLeast(0),
            modifier = Modifier.fillMaxWidth()
        )
    }
}

@Composable
private fun IntStepper(
    label: String,
    value: Int,
    step: Int,
    onValueChange: (Int) -> Unit
) {
    Row(modifier = Modifier.fillMaxWidth()) {
        Button(onClick = { onValueChange(value - step) }) {
            Text("-")
        }
        Text(
            text = "$label: $value",
            modifier = Modifier
                .weight(1f)
                .padding(12.dp)
        )
        Button(onClick = { onValueChange(value + step) }) {
            Text("+")
        }
    }
}

@Composable
private fun ToggleSetting(
    label: String,
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit
) {
    Row(modifier = Modifier.fillMaxWidth()) {
        Text(
            text = label,
            modifier = Modifier
                .weight(1f)
                .padding(vertical = 12.dp)
        )
        Switch(checked = checked, onCheckedChange = onCheckedChange)
    }
}

@Composable
private fun <T> NullableBlock(
    label: String,
    value: T?,
    defaultValue: T,
    onValueChange: (T?) -> Unit,
    content: @Composable ColumnScope.(T, (T) -> Unit) -> Unit
) {
    Section(label) {
        ToggleSetting("Enabled", value != null) { enabled ->
            onValueChange(if (enabled) value ?: defaultValue else null)
        }
        value?.let {
            content(it) { updated -> onValueChange(updated) }
        }
    }
}

@Composable
private fun FbmBlock(
    label: String,
    value: NtscSettings.FbmNoise?,
    defaultValue: NtscSettings.FbmNoise,
    onValueChange: (NtscSettings.FbmNoise?) -> Unit
) {
    NullableBlock(
        label = label,
        value = value,
        defaultValue = defaultValue,
        onValueChange = onValueChange
    ) { noise, update ->
        FloatSlider("Frequency", noise.frequency, 0f..2f) {
            update(noise.copy(frequency = it))
        }
        FloatSlider("Intensity", noise.intensity, 0f..1f) {
            update(noise.copy(intensity = it))
        }
        IntSlider("Detail", noise.detail, 0..5) {
            update(noise.copy(detail = it))
        }
    }
}

private fun Float.shortString(): String {
    val rounded = (this * 10_000f).roundToInt() / 10_000f
    return rounded.toString()
}

class GenericTransformation(
    val key: Any? = Random.nextInt(),
    val action: suspend (Bitmap, Size) -> Bitmap
) : Transformation {

    constructor(
        key: Any? = Random.nextInt(),
        action: suspend (Bitmap) -> Bitmap
    ) : this(
        key, { t, _ -> action(t) }
    )

    override val cacheKey: String
        get() = (action to key).hashCode().toString()

    override suspend fun transform(
        input: Bitmap,
        size: Size
    ): Bitmap = action(input, size)
}

fun mapToBitmap(
    touch: Offset,
    imageSize: IntSize,
    bitmap: Bitmap
): Offset {
    val scaleX = bitmap.width.toFloat() / imageSize.width
    val scaleY = bitmap.height.toFloat() / imageSize.height
    return Offset(
        touch.x * scaleX,
        touch.y * scaleY
    )
}
