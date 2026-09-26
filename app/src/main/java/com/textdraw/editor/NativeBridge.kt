package com.textdraw.editor

import android.content.res.AssetManager

object NativeBridge {
    init {
        System.loadLibrary("textdraw_editor")
    }

    external fun nativeInit(assetManager: AssetManager, storagePath: String)
    external fun nativeSurfaceCreated()
    external fun nativeSurfaceChanged(width: Int, height: Int, density: Float)
    external fun nativeDrawFrame()
    external fun nativeTouchEvent(action: Int, x: Float, y: Float, pointerCount: Int)
    external fun nativePinchZoom(scaleFactor: Float)
    external fun nativeInputCharacters(text: String)
    external fun nativeInputKey(keyCode: Int, isDown: Boolean)
    external fun nativeSetDialogText(fieldId: Int, text: String)
    external fun nativeExportPawn(): String
    external fun nativeImportPawn(code: String)
    external fun nativeOnBackPressed(): Boolean
}
