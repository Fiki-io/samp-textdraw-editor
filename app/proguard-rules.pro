# Keep native JNI methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep NativeBridge
-keep class com.textdraw.editor.NativeBridge { *; }
