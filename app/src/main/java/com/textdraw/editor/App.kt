package com.textdraw.editor

import android.app.Application

class App : Application() {
    override fun onCreate() {
        super.onCreate()
        // Initialize Global Crash Handler before anything else
        CrashHandler.init(this)
    }
}
