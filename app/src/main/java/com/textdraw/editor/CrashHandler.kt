package com.textdraw.editor

import android.app.Application
import android.content.Context
import android.content.Intent
import android.os.Build
import android.os.Process
import android.util.Log
import java.io.File
import java.io.PrintWriter
import java.io.StringWriter
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class CrashHandler private constructor(private val context: Context) : Thread.UncaughtExceptionHandler {

    private val defaultHandler: Thread.UncaughtExceptionHandler? = Thread.getDefaultUncaughtExceptionHandler()

    override fun uncaughtException(thread: Thread, throwable: Throwable) {
        handleCrash(thread, throwable)
    }

    private fun handleCrash(thread: Thread, throwable: Throwable) {
        try {
            val report = buildCrashReport(thread, throwable)
            Log.e("CrashHandler", report)

            val logFile = saveCrashReport(report)

            val intent = Intent(context, CrashActivity::class.java).apply {
                putExtra(CrashActivity.EXTRA_CRASH_LOG, report)
                putExtra(CrashActivity.EXTRA_LOG_PATH, logFile.absolutePath)
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
            }
            context.startActivity(intent)

            // Kill the crashed process
            Process.killProcess(Process.myPid())
            System.exit(10)
        } catch (e: Exception) {
            Log.e("CrashHandler", "Failed to handle uncaught exception", e)
            defaultHandler?.uncaughtException(thread, throwable)
        }
    }

    private fun buildCrashReport(thread: Thread, throwable: Throwable): String {
        val sw = StringWriter()
        val pw = PrintWriter(sw)
        throwable.printStackTrace(pw)
        val stackTrace = sw.toString()

        val timeStr = SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", Locale.US).format(Date())
        val versionName = try {
            val pInfo = context.packageManager.getPackageInfo(context.packageName, 0)
            pInfo.versionName
        } catch (e: Exception) {
            "1.0.0"
        }

        return buildString {
            appendLine("==================================================")
            appendLine("       SA-MP TEXTDRAW EDITOR CRASH REPORT        ")
            appendLine("==================================================")
            appendLine("Waktu Crash   : $timeStr")
            appendLine("Versi App     : $versionName")
            appendLine("Perangkat     : ${Build.MANUFACTURER} ${Build.MODEL} (${Build.DEVICE})")
            appendLine("Hardware      : ${Build.HARDWARE} / ${Build.BOARD}")
            appendLine("Android OS    : Android ${Build.VERSION.RELEASE} (API ${Build.VERSION.SDK_INT})")
            appendLine("Supported ABIs: ${Build.SUPPORTED_ABIS.joinToString(", ")}")
            appendLine("Crashing Thread: ${thread.name} (id: ${thread.id})")
            appendLine("--------------------------------------------------")
            appendLine("EXCEPTION TYPE:")
            appendLine(throwable.javaClass.name)
            appendLine("MESSAGE:")
            appendLine(throwable.message ?: "(pesan error kosong)")
            appendLine("--------------------------------------------------")
            appendLine("STACKTRACE:")
            appendLine(stackTrace.trim())
            appendLine("==================================================")
        }
    }

    private fun saveCrashReport(report: String): File {
        val dir = File(context.filesDir, "crash_logs")
        if (!dir.exists()) dir.mkdirs()

        val lastCrash = File(dir, "last_crash.txt")
        lastCrash.writeText(report)

        val timestamp = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
        val historyFile = File(dir, "crash_$timestamp.txt")
        historyFile.writeText(report)

        return lastCrash
    }

    companion object {
        private var instance: CrashHandler? = null

        @JvmStatic
        fun init(application: Application) {
            if (instance == null) {
                instance = CrashHandler(application.applicationContext)
                Thread.setDefaultUncaughtExceptionHandler(instance)
            }
        }

        @JvmStatic
        fun handleManualException(context: Context, throwable: Throwable) {
            val handler = instance ?: CrashHandler(context.applicationContext)
            handler.handleCrash(Thread.currentThread(), throwable)
        }
    }
}
