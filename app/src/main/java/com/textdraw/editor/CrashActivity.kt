package com.textdraw.editor

import android.app.Activity
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.os.Process
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import java.io.File

class CrashActivity : Activity() {

    private lateinit var tvCrashLog: TextView
    private lateinit var btnCopyLog: Button
    private lateinit var btnRestart: Button
    private lateinit var btnClose: Button

    private var crashLogText: String = ""

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_crash)

        tvCrashLog = findViewById(R.id.tv_crash_log)
        btnCopyLog = findViewById(R.id.btn_copy_log)
        btnRestart = findViewById(R.id.btn_restart)
        btnClose = findViewById(R.id.btn_close)

        // Read log from intent or file
        crashLogText = intent.getStringExtra(EXTRA_CRASH_LOG) ?: ""
        if (crashLogText.isEmpty()) {
            val path = intent.getStringExtra(EXTRA_LOG_PATH)
            if (!path.isNullOrEmpty()) {
                val f = File(path)
                if (f.exists()) {
                    crashLogText = f.readText()
                }
            }
        }

        // Fallback: check default crash log path
        if (crashLogText.isEmpty()) {
            val fallback = File(filesDir, "crash_logs/last_crash.txt")
            if (fallback.exists()) {
                crashLogText = fallback.readText()
            }
        }

        if (crashLogText.isEmpty()) {
            crashLogText = "Tidak ada detail log yang tersedia."
        }

        tvCrashLog.text = crashLogText

        // Copy log button
        btnCopyLog.setOnClickListener {
            val clipboard = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager
            val clip = ClipData.newPlainText("SA-MP TextDraw Crash Log", crashLogText)
            clipboard.setPrimaryClip(clip)
            Toast.makeText(this, "✅ Log crash berhasil disalin ke clipboard!", Toast.LENGTH_LONG).show()
        }

        // Restart application button
        btnRestart.setOnClickListener {
            val intent = Intent(this, MainActivity::class.java).apply {
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
            }
            startActivity(intent)
            finish()
            Process.killProcess(Process.myPid())
        }

        // Close button
        btnClose.setOnClickListener {
            finishAffinity()
            Process.killProcess(Process.myPid())
            System.exit(0)
        }
    }

    companion object {
        const val EXTRA_CRASH_LOG = "extra_crash_log"
        const val EXTRA_LOG_PATH = "extra_log_path"
    }
}
