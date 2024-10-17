package com.t8rin.trickle.app

import android.graphics.Bitmap
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.PickVisualMediaRequest
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Slider
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.derivedStateOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import coil.imageLoader
import coil.request.ImageRequest
import coil.size.Size
import coil.transform.Transformation
import coil.util.DebugLogger
import com.t8rin.trickle.Trickle
import com.t8rin.trickle.TrickleUtils.generateShades
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
            Column(
                modifier = Modifier
                    .weight(1f)
                    .verticalScroll(rememberScrollState())
            ) {
                AsyncImage(
                    model = remember(source, target, intensity, colorValue, colors) {
                        ImageRequest.Builder(this@Jp2Hypothesis).allowHardware(false)
                            .data(source)
                            .transformations(
                                if (intensity == 1f) {
                                    listOf(
                                        GenericTransformation { bmp ->
                                            List(4) { it }.fold(bmp) { acc, i ->
                                                when (i % 5) {
                                                    0 -> Trickle.hdr(acc)
                                                    1 -> Trickle.tv(acc)
                                                    2 -> Trickle.oil(acc, 5)
                                                    3 -> Trickle.sketch(acc)
                                                    else -> Trickle.gotham(acc)
                                                }
                                            }
                                        }
                                    )
                                } else emptyList()
                            ).build()
                    },
                    imageLoader = imageLoader,
                    modifier = Modifier.height(300.dp),
                    contentDescription = null
                )
//                DitheringType.entries.forEach { type ->
//                    Text(type.name)
//                    AsyncImage(
//                        model = remember(source, isGray, colors) {
//                            ImageRequest.Builder(this@Jp2Hypothesis).allowHardware(false)
//                                .data(source)
//                                .transformations(
//                                    GenericTransformation { bmp ->
//                                        Trickle.dithering(
//                                            input = bmp,
//                                            type = type,
//                                            threshold = 128,
//                                            isGrayScale = isGray
//                                        )
//                                    }
//                                ).build()
//                        },
//                        imageLoader = imageLoader,
//                        modifier = Modifier.height(300.dp),
//                        contentDescription = null
//                    )
//                }
            }
            Column(
                modifier = Modifier
                    .weight(1f)
                    .verticalScroll(rememberScrollState())
            ) {
//                DitherTool.Type.entries.forEach { type ->
//                    Text(type.name)
//                    AsyncImage(
//                        model = remember(source, isGray, colors) {
//                            ImageRequest.Builder(this@Jp2Hypothesis).allowHardware(false)
//                                .data(source)
//                                .transformations(
//                                    GenericTransformation { bmp ->
//                                        DitherTool(
//                                            threshold = 128,
//                                            isGrayScale = isGray
//                                        ).dither(
//                                            type = type,
//                                            src = bmp
//                                        )
//                                    }
//                                ).build()
//                        },
//                        imageLoader = imageLoader,
//                        modifier = Modifier.height(300.dp),
//                        contentDescription = null
//                    )
//                }
            }
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