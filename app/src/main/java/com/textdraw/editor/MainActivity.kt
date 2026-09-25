package com.textdraw.editor

import android.app.Activity
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.widget.Toast
import java.io.File
import java.lang.ref.WeakReference

class MainActivity : Activity() {

    private lateinit var glSurfaceView: EditorGLSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        instance = WeakReference(this)

        // Keep screen on while editing
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        // Hide status & navigation bars for immersive widescreen
        hideSystemUI()

        // Ensure projects directory exists in app storage
        val projectsDir = File(getExternalFilesDir(null), "projects")
        if (!projectsDir.exists()) {
            projectsDir.mkdirs()
        }

        // Initialize Native C++ engine with APK assets and storage path
        NativeBridge.nativeInit(assets, projectsDir.absolutePath)

        // Set GL surface
        glSurfaceView = EditorGLSurfaceView(this)
        setContentView(glSurfaceView)
    }

    override fun onResume() {
        super.onResume()
        hideSystemUI()
        glSurfaceView.onResume()
    }

    override fun onPause() {
        super.onPause()
        glSurfaceView.onPause()
    }

    private fun hideSystemUI() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.insetsController?.let { controller ->
                controller.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                controller.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            )
        }
    }

    companion object {
        private var instance: WeakReference<MainActivity>? = null

        @JvmStatic
        fun copyToClipboard(text: String) {
            instance?.get()?.let { activity ->
                activity.runOnUiThread {
                    val clipboard = activity.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                    val clip = ClipData.newPlainText("SA-MP Pawn Code", text)
                    clipboard.setPrimaryClip(clip)
                    Toast.makeText(activity, "Pawn code copied to clipboard!", Toast.LENGTH_SHORT).show()
                }
            }
        }

        @JvmStatic
        fun getFromClipboard(): String {
            var result = ""
            instance?.get()?.let { activity ->
                val clipboard = activity.getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
                val clip = clipboard.primaryClip
                if (clip != null && clip.itemCount > 0) {
                    result = clip.getItemAt(0).text?.toString() ?: ""
                }
            }
            return result
        }

        @JvmStatic
        fun showToast(message: String) {
            instance?.get()?.let { activity ->
                activity.runOnUiThread {
                    Toast.makeText(activity, message, Toast.LENGTH_SHORT).show()
                }
            }
        }
    }
}
