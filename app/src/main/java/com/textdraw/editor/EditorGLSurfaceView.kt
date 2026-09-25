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

    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        try {
            NativeBridge.nativeSurfaceCreated()
        } catch (t: Throwable) {
            CrashHandler.handleManualException(context, t)
        }
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        try {
            NativeBridge.nativeSurfaceChanged(width, height)
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
