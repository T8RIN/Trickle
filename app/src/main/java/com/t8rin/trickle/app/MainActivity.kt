package com.t8rin.trickle.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.systemBarsPadding
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier
import com.t8rin.trickle.app.ui.theme.TrickleTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            TrickleTheme {
                Surface {
                    Box(modifier = Modifier.systemBarsPadding()) {
                        Jp2Hypothesis()
                    }
                }
            }
        }
    }
}