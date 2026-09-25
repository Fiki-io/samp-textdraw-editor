package com.textdraw.editor

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class EditorGLSurfaceView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : GLSurfaceView(context, attrs), GLSurfaceView.Renderer {

    private val scaleDetector: ScaleGestureDetector

    init {
        setEGLContextClientVersion(3)
        setRenderer(this)
        renderMode = RENDERMODE_CONTINUOUSLY
        preserveEGLContextOnPause = true

        isFocusable = true
        isFocusableInTouchMode = true

        scaleDetector = ScaleGestureDetector(context, object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
            override fun onScale(detector: ScaleGestureDetector): Boolean {
                val factor = detector.scaleFactor
                queueEvent {
                    NativeBridge.nativePinchZoom(factor)
                }
                return true
            }
        })
    }

    override fun onCreateInputConnection(outAttrs: android.view.inputmethod.EditorInfo): android.view.inputmethod.InputConnection {
        outAttrs.inputType = android.text.InputType.TYPE_CLASS_TEXT or android.text.InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
        outAttrs.imeOptions = android.view.inputmethod.EditorInfo.IME_ACTION_DONE or android.view.inputmethod.EditorInfo.IME_FLAG_NO_FULLSCREEN
        return object : android.view.inputmethod.BaseInputConnection(this, false) {
            override fun commitText(text: CharSequence?, newCursorPosition: Int): Boolean {
                text?.let { str ->
                    queueEvent {
                        NativeBridge.nativeInputCharacters(str.toString())
                    }
                }
                return true
            }

            override fun deleteSurroundingText(beforeLength: Int, afterLength: Int): Boolean {
                if (beforeLength > 0) {
                    queueEvent {
                        NativeBridge.nativeInputKey(67) // Android KEYCODE_DEL
                    }
                }
                return super.deleteSurroundingText(beforeLength, afterLength)
            }
        }
    }

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        try {
            NativeBridge.nativeSurfaceCreated()
        } catch (t: Throwable) {
            CrashHandler.handleManualException(context, t)
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        try {
            val density = resources.displayMetrics.density
            NativeBridge.nativeSurfaceChanged(width, height, density)
        } catch (t: Throwable) {
            CrashHandler.handleManualException(context, t)
        }
    }

    override fun onDrawFrame(gl: GL10?) {
        try {
            NativeBridge.nativeDrawFrame()
        } catch (t: Throwable) {
            CrashHandler.handleManualException(context, t)
        }
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        scaleDetector.onTouchEvent(event)

        val action = event.actionMasked
        val pointerCount = event.pointerCount
        val x = event.x
        val y = event.y

        queueEvent {
            try {
                NativeBridge.nativeTouchEvent(action, x, y, pointerCount)
            } catch (t: Throwable) {
                CrashHandler.handleManualException(context, t)
            }
        }
        return true
    }
}
