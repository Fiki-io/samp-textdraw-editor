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
            // Keep screen on while editing and configure edge-to-edge layout
            window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                window.attributes.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
            }
            window.setFlags(
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS
            )

            // Hide status & navigation bars for immersive widescreen
            hideSystemUI()

            // Initialize GL Surface and set content view
            glSurfaceView = EditorGLSurfaceView(this)
            setContentView(glSurfaceView)

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

    private var lastBackPressTime = 0L

    @Deprecated("Deprecated in Java")
    override fun onBackPressed() {
        if (!::glSurfaceView.isInitialized) {
            super.onBackPressed()
            return
        }

        // Ask native engine if any modal, picker, or active selection is open
        val future = java.util.concurrent.FutureTask {
            NativeBridge.nativeOnBackPressed()
        }
        glSurfaceView.queueEvent(future)
        val handled = try {
            future.get(300, java.util.concurrent.TimeUnit.MILLISECONDS) ?: false
        } catch (_: Throwable) {
            false
        }

        if (handled) {
            // A dialog/picker or active selection was closed safely
            return
        }

        // Double-tap back within 2 seconds to exit to prevent accidental loss of work
        val currentTime = System.currentTimeMillis()
        if (currentTime - lastBackPressTime < 2000L) {
            super.onBackPressed()
        } else {
            lastBackPressTime = currentTime
            Toast.makeText(this, "Tekan sekali lagi untuk keluar", Toast.LENGTH_SHORT).show()
        }
    }

    companion object {
        private var instance: WeakReference<MainActivity>? = null

        @JvmStatic
        fun shareText(text: String, title: String) {
            instance?.get()?.let { activity ->
                activity.runOnUiThread {
                    val intent = android.content.Intent(android.content.Intent.ACTION_SEND).apply {
                        type = "text/plain"
                        putExtra(android.content.Intent.EXTRA_SUBJECT, title)
                        putExtra(android.content.Intent.EXTRA_TEXT, text)
                    }
                    activity.startActivity(android.content.Intent.createChooser(intent, title))
                }
            }
        }

        @JvmStatic
        fun savePawnFile(filename: String, content: String): Boolean {
            val activity = instance?.get() ?: return false
            return try {
                val cleanName = if (filename.endsWith(".pwn", ignoreCase = true)) filename else "$filename.pwn"
                val projectsDir = File(activity.getExternalFilesDir(null), "projects")
                if (!projectsDir.exists()) projectsDir.mkdirs()
                val file = File(projectsDir, cleanName)
                file.writeText(content)
                activity.runOnUiThread {
                    Toast.makeText(activity, "Berhasil disimpan ke: ${file.name} di storage!", Toast.LENGTH_LONG).show()
                }
                true
            } catch (t: Throwable) {
                activity.runOnUiThread {
                    Toast.makeText(activity, "Gagal menyimpan file .pwn: ${t.message}", Toast.LENGTH_SHORT).show()
                }
                false
            }
        }

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
            val activity = instance?.get() ?: return ""
            try {
                if (android.os.Looper.myLooper() == android.os.Looper.getMainLooper()) {
                    val clipboard = activity.getSystemService(Context.CLIPBOARD_SERVICE) as? ClipboardManager
                    val clip = clipboard?.primaryClip
                    if (clip != null && clip.itemCount > 0) {
                        result = clip.getItemAt(0).coerceToText(activity).toString()
                    }
                } else {
                    val future = java.util.concurrent.FutureTask {
                        val clipboard = activity.getSystemService(Context.CLIPBOARD_SERVICE) as? ClipboardManager
                        val clip = clipboard?.primaryClip
                        if (clip != null && clip.itemCount > 0) {
                            clip.getItemAt(0).coerceToText(activity).toString()
                        } else {
                            ""
                        }
                    }
                    activity.runOnUiThread(future)
                    result = future.get(500, java.util.concurrent.TimeUnit.MILLISECONDS) ?: ""
                }
            } catch (e: Throwable) {
                android.util.Log.e("MainActivity", "Error getting clipboard: ${e.message}")
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
                    if (activity.isFinishing) return@runOnUiThread
                    val imm = activity.getSystemService(Context.INPUT_METHOD_SERVICE) as? android.view.inputmethod.InputMethodManager ?: return@runOnUiThread
                    val view = activity.glSurfaceView

                    if (visible) {
                        view.isFocusable = true
                        view.isFocusableInTouchMode = true
                        view.requestFocus()
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                            val controller = activity.window?.insetsController
                            if (controller != null) {
                                controller.show(WindowInsets.Type.ime())
                            } else {
                                imm.showSoftInput(view, android.view.inputmethod.InputMethodManager.SHOW_IMPLICIT)
                            }
                        } else {
                            imm.showSoftInput(view, android.view.inputmethod.InputMethodManager.SHOW_IMPLICIT)
                        }
                    } else {
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                            val controller = activity.window?.insetsController
                            if (controller != null) {
                                controller.hide(WindowInsets.Type.ime())
                            } else {
                                imm.hideSoftInputFromWindow(view.windowToken, 0)
                            }
                        } else {
                            imm.hideSoftInputFromWindow(view.windowToken, 0)
                        }
                    }
                }
            }
        }

        @JvmStatic
        fun showTextInputDialog(title: String, initialText: String, fieldId: Int) {
            instance?.get()?.let { activity ->
                activity.runOnUiThread {
                    if (activity.isFinishing) return@runOnUiThread
                    val isMultiline = (fieldId == 1 || fieldId == 5)
                    val dp = activity.resources.displayMetrics.density

                    val root = android.widget.LinearLayout(activity).apply {
                        orientation = android.widget.LinearLayout.VERTICAL
                        setPadding((16 * dp).toInt(), (12 * dp).toInt(), (16 * dp).toInt(), (8 * dp).toInt())
                        setBackgroundColor(android.graphics.Color.parseColor("#1C1E24"))
                    }

                    // Top quick action row: Paste & Clear All
                    val actionRow = android.widget.LinearLayout(activity).apply {
                        orientation = android.widget.LinearLayout.HORIZONTAL
                        gravity = android.view.Gravity.CENTER_VERTICAL
                        layoutParams = android.widget.LinearLayout.LayoutParams(
                            android.widget.LinearLayout.LayoutParams.MATCH_PARENT,
                            android.widget.LinearLayout.LayoutParams.WRAP_CONTENT
                        ).apply { bottomMargin = (8 * dp).toInt() }
                    }

                    val titleView = android.widget.TextView(activity).apply {
                        text = title
                        textSize = 15f
                        setTextColor(android.graphics.Color.parseColor("#FFBF00"))
                        typeface = android.graphics.Typeface.DEFAULT_BOLD
                        layoutParams = android.widget.LinearLayout.LayoutParams(
                            0,
                            android.widget.LinearLayout.LayoutParams.WRAP_CONTENT,
                            1f
                        )
                    }
                    actionRow.addView(titleView)

                    val btnPaste = android.widget.Button(activity, null, android.R.attr.buttonStyleSmall).apply {
                        text = "Tempel"
                        textSize = 11f
                        setTextColor(android.graphics.Color.WHITE)
                        setBackgroundColor(android.graphics.Color.parseColor("#2C313C"))
                    }
                    actionRow.addView(btnPaste)

                    val btnClear = android.widget.Button(activity, null, android.R.attr.buttonStyleSmall).apply {
                        text = "Hapus"
                        textSize = 11f
                        setTextColor(android.graphics.Color.parseColor("#E74C3C"))
                        setBackgroundColor(android.graphics.Color.parseColor("#2C313C"))
                        layoutParams = android.widget.LinearLayout.LayoutParams(
                            android.widget.LinearLayout.LayoutParams.WRAP_CONTENT,
                            android.widget.LinearLayout.LayoutParams.WRAP_CONTENT
                        ).apply { leftMargin = (6 * dp).toInt() }
                    }
                    actionRow.addView(btnClear)
                    root.addView(actionRow)

                    val input = android.widget.EditText(activity).apply {
                        setText(initialText)
                        setSelection(text.length)
                        setTextColor(android.graphics.Color.WHITE)
                        setHintTextColor(android.graphics.Color.parseColor("#7F8C8D"))
                        hint = if (fieldId == 1) "Ketik teks TextDraw di sini..." else "Ketik di sini..."
                        textSize = 15f
                        setBackgroundColor(android.graphics.Color.parseColor("#141619"))
                        setPadding((12 * dp).toInt(), (12 * dp).toInt(), (12 * dp).toInt(), (12 * dp).toInt())
                        if (isMultiline) {
                            isSingleLine = false
                            minLines = 3
                            maxLines = 8
                            gravity = android.view.Gravity.TOP or android.view.Gravity.START
                        } else {
                            isSingleLine = true
                        }
                    }

                    // SA-MP color tag chips toolbar for text editing (fieldId == 1)
                    if (fieldId == 1) {
                        val scroll = android.widget.HorizontalScrollView(activity).apply {
                            isHorizontalScrollBarEnabled = false
                            layoutParams = android.widget.LinearLayout.LayoutParams(
                                android.widget.LinearLayout.LayoutParams.MATCH_PARENT,
                                android.widget.LinearLayout.LayoutParams.WRAP_CONTENT
                            ).apply { bottomMargin = (8 * dp).toInt() }
                        }
                        val tagLayout = android.widget.LinearLayout(activity).apply {
                            orientation = android.widget.LinearLayout.HORIZONTAL
                        }

                        val tags = listOf(
                            Pair("~r~ Merah", "~r~"),
                            Pair("~g~ Hijau", "~g~"),
                            Pair("~b~ Biru", "~b~"),
                            Pair("~y~ Kuning", "~y~"),
                            Pair("~w~ Putih", "~w~"),
                            Pair("~p~ Ungu", "~p~"),
                            Pair("~l~ Hitam", "~l~"),
                            Pair("~n~ [Enter]", "~n~"),
                            Pair("~<~ Panah Kiri", "~<~"),
                            Pair("~>~ Panah Kanan", "~>~"),
                            Pair("~u~ Panah Atas", "~u~"),
                            Pair("~d~ Panah Bawah", "~d~")
                        )

                        for ((label, tag) in tags) {
                            val tagBtn = android.widget.TextView(activity).apply {
                                text = label
                                textSize = 11f
                                setTextColor(android.graphics.Color.WHITE)
                                setBackgroundColor(android.graphics.Color.parseColor("#272B34"))
                                setPadding((10 * dp).toInt(), (6 * dp).toInt(), (10 * dp).toInt(), (6 * dp).toInt())
                                layoutParams = android.widget.LinearLayout.LayoutParams(
                                    android.widget.LinearLayout.LayoutParams.WRAP_CONTENT,
                                    android.widget.LinearLayout.LayoutParams.WRAP_CONTENT
                                ).apply { rightMargin = (6 * dp).toInt() }
                                setOnClickListener {
                                    val start = Math.max(input.selectionStart, 0)
                                    val end = Math.max(input.selectionEnd, 0)
                                    input.text.replace(Math.min(start, end), Math.max(start, end), tag, 0, tag.length)
                                }
                            }
                            tagLayout.addView(tagBtn)
                        }
                        scroll.addView(tagLayout)
                        root.addView(scroll)
                    }

                    btnPaste.setOnClickListener {
                        val clipText = getFromClipboard()
                        if (clipText.isNotEmpty()) {
                            val start = Math.max(input.selectionStart, 0)
                            val end = Math.max(input.selectionEnd, 0)
                            input.text.replace(Math.min(start, end), Math.max(start, end), clipText, 0, clipText.length)
                        }
                    }

                    btnClear.setOnClickListener {
                        input.setText("")
                    }

                    root.addView(input)

                    val dialog = android.app.AlertDialog.Builder(activity, android.R.style.Theme_DeviceDefault_Dialog_Alert)
                        .setView(root)
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
