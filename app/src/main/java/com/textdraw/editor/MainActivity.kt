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

        try {
            // Keep screen on while editing
            window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

            // Initialize GL Surface and set content view first so DecorView is created
            glSurfaceView = EditorGLSurfaceView(this)
            setContentView(glSurfaceView)

            // Hide status & navigation bars for immersive widescreen
            hideSystemUI()

            // Ensure projects directory exists in app storage
            val projectsDir = File(getExternalFilesDir(null), "projects")
            if (!projectsDir.exists()) {
                projectsDir.mkdirs()
            }

            // Initialize Native C++ engine with APK assets and storage path
            NativeBridge.nativeInit(assets, projectsDir.absolutePath)
        } catch (t: Throwable) {
            CrashHandler.handleManualException(this, t)
        }
    }

    override fun onResume() {
        super.onResume()
        hideSystemUI()
        if (::glSurfaceView.isInitialized) {
            glSurfaceView.onResume()
        }
    }

    override fun onPause() {
        super.onPause()
        if (::glSurfaceView.isInitialized) {
            glSurfaceView.onPause()
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUI()
        }
    }

    private fun hideSystemUI() {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                val decor = window?.peekDecorView() ?: window?.decorView
                decor?.windowInsetsController?.let { controller ->
                    controller.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                    controller.systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
                    return
                }
            }
        } catch (_: Throwable) {
            // Fallback gracefully on custom vendor ROMs if insetsController fails
        }

        try {
            @Suppress("DEPRECATION")
            window?.decorView?.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            )
        } catch (_: Throwable) {
            // Ignore if window is detached
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

        @JvmStatic
        fun setKeyboardVisible(visible: Boolean) {
            instance?.get()?.let { activity ->
                activity.runOnUiThread {
                    val imm = activity.getSystemService(Context.INPUT_METHOD_SERVICE) as? android.view.inputmethod.InputMethodManager ?: return@runOnUiThread
                    if (activity.isFinishing) return@runOnUiThread
                    if (visible) {
                        imm.showSoftInput(activity.glSurfaceView, android.view.inputmethod.InputMethodManager.SHOW_IMPLICIT)
                    } else {
                        imm.hideSoftInputFromWindow(activity.glSurfaceView.windowToken, 0)
                    }
                }
            }
        }

        @JvmStatic
        fun showTextInputDialog(title: String, initialText: String, fieldId: Int) {
            instance?.get()?.let { activity ->
                activity.runOnUiThread {
                    if (activity.isFinishing) return@runOnUiThread
                    val input = android.widget.EditText(activity).apply {
                        setText(initialText)
                        setSelection(text.length)
                        setTextColor(android.graphics.Color.WHITE)
                        setBackgroundColor(android.graphics.Color.parseColor("#22242A"))
                        setPadding(32, 24, 32, 24)
                    }
                    val container = android.widget.FrameLayout(activity).apply {
                        setPadding(40, 20, 40, 10)
                        addView(input)
                    }
                    val dialog = android.app.AlertDialog.Builder(activity, android.R.style.Theme_DeviceDefault_Dialog_Alert)
                        .setTitle(title)
                        .setView(container)
                        .setPositiveButton("Simpan") { _, _ ->
                            val result = input.text.toString()
                            activity.glSurfaceView.queueEvent {
                                NativeBridge.nativeSetDialogText(fieldId, result)
                            }
                        }
                        .setNegativeButton("Batal", null)
                        .create()

                    dialog.window?.setSoftInputMode(android.view.WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE)
                    dialog.show()
                    input.requestFocus()
                }
            }
        }
    }
}
