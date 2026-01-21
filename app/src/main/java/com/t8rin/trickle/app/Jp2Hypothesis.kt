package com.t8rin.trickle.app

import android.graphics.Bitmap
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.PickVisualMediaRequest
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
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
import coil.compose.AsyncImage
import coil.imageLoader
import coil.request.ImageRequest
import coil.size.Size
import coil.transform.Transformation
import coil.util.DebugLogger
import com.t8rin.trickle.TrickleUtils.generateShades
import com.t8rin.trickle.WarpBrush
import com.t8rin.trickle.WarpEngine
import com.t8rin.trickle.WarpMode
import kotlin.random.Random


@Composable
fun MainActivity.Jp2Hypothesis() {
    var source by remember {
        mutableStateOf("")
    }

    var target by remember {
        mutableStateOf<String>("")
    }


    val imagePicker =
        rememberLauncherForActivityResult(contract = ActivityResultContracts.PickVisualMedia()) {
            source = it?.toString() ?: ""
        }

    val pickImage: () -> Unit = {
        imagePicker.launch(PickVisualMediaRequest())
    }

    val imagePicker2 =
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


    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(24.dp)
    ) {
        val imageLoader = remember {
            imageLoader.newBuilder().logger(DebugLogger()).build()
        }
        var isGray by remember {
            mutableStateOf(true)
        }
        Row(
            modifier = Modifier.weight(1f)
        ) {
            AsyncImage(
                model = source,
                imageLoader = imageLoader,
                modifier = Modifier.weight(1f),
                contentDescription = null
            )
            AsyncImage(
                model = target,
                imageLoader = imageLoader,
                modifier = Modifier.weight(1f),
                contentDescription = null
            )
            Switch(checked = isGray, onCheckedChange = { isGray = it })
        }

        val colors by remember {
            derivedStateOf {
                generateShades(
                    Color.Cyan.toArgb(),
                    shadeStep = (20).toInt().coerceAtLeast(2)
                )
            }
        }

        Row(
            modifier = Modifier.weight(1f)
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
                                    WarpMode.SWIRL_CW
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