# Keep native JNI methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep all app classes, activities, and bridges
-keep class com.textdraw.editor.** { *; }
-dontwarn com.textdraw.editor.**
