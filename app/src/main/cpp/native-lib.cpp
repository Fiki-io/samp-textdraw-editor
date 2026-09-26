#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>

#include "engine/TextDrawManager.h"
#include "engine/Viewport.h"
#include "engine/AssetManager.h"
#include "engine/DffRenderer.h"
#include "engine/PawnExporter.h"
#include "ui/EditorUI.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_opengl3.h"
#include "utils/CrashHandlerNative.h"

#define LOG_TAG "TextDraw_Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static TextDrawManager g_manager;
static Viewport g_viewport;

static JavaVM* g_vm = nullptr;
static std::string g_storage_path = "";

static float g_last_touch_x = 0.0f;
static float g_last_touch_y = 0.0f;
static bool g_is_dragging_textdraw = false;
static bool g_is_panning = false;
static bool g_is_touching_ui = false;

static void android_set_clipboard(const char* text) {
    if (!g_vm || !text) return;
    JNIEnv* env = nullptr;
    if (g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK || !env) return;
    
    jclass main_cls = env->FindClass("com/textdraw/editor/MainActivity");
    if (main_cls) {
        jmethodID copy_mid = env->GetStaticMethodID(main_cls, "copyToClipboard", "(Ljava/lang/String;)V");
        if (copy_mid) {
            jstring j_str = env->NewStringUTF(text);
            env->CallStaticVoidMethod(main_cls, copy_mid, j_str);
            env->DeleteLocalRef(j_str);
        }
        env->DeleteLocalRef(main_cls);
    }
}

std::string android_get_clipboard() {
    std::string result = "";
    if (!g_vm) return result;
    JNIEnv* env = nullptr;
    if (g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK || !env) return result;
    
    jclass main_cls = env->FindClass("com/textdraw/editor/MainActivity");
    if (main_cls) {
        jmethodID get_mid = env->GetStaticMethodID(main_cls, "getFromClipboard", "()Ljava/lang/String;");
        if (get_mid) {
            jstring j_str = (jstring)env->CallStaticObjectMethod(main_cls, get_mid);
            if (j_str) {
                const char* utf = env->GetStringUTFChars(j_str, nullptr);
                if (utf) {
                    result = utf;
                    env->ReleaseStringUTFChars(j_str, utf);
                }
                env->DeleteLocalRef(j_str);
            }
        }
        env->DeleteLocalRef(main_cls);
    }
    return result;
}

static void android_set_keyboard_visible(bool visible) {
    if (!g_vm) return;
    JNIEnv* env = nullptr;
    if (g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK || !env) return;
    
    jclass main_cls = env->FindClass("com/textdraw/editor/MainActivity");
    if (main_cls) {
        jmethodID mid = env->GetStaticMethodID(main_cls, "setKeyboardVisible", "(Z)V");
        if (mid) {
            env->CallStaticVoidMethod(main_cls, mid, (jboolean)visible);
        }
        env->DeleteLocalRef(main_cls);
    }
}

void android_show_text_dialog(const char* title, const char* initial_text, int field_id) {
    if (!g_vm) return;
    JNIEnv* env = nullptr;
    if (g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK || !env) return;
    
    jclass main_cls = env->FindClass("com/textdraw/editor/MainActivity");
    if (main_cls) {
        jmethodID mid = env->GetStaticMethodID(main_cls, "showTextInputDialog", "(Ljava/lang/String;Ljava/lang/String;I)V");
        if (mid) {
            jstring j_title = env->NewStringUTF(title ? title : "");
            jstring j_text = env->NewStringUTF(initial_text ? initial_text : "");
            env->CallStaticVoidMethod(main_cls, mid, j_title, j_text, (jint)field_id);
            env->DeleteLocalRef(j_title);
            env->DeleteLocalRef(j_text);
        }
        env->DeleteLocalRef(main_cls);
    }
}

static bool is_touch_over_ui(float x, float y) {
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (!g) return false;
    
    ImVec2 pt(x, y);
    
    // Top menu bar
    if (y < 42.0f * EditorUI::get().get_ui_scale()) return true;
    
    // Check all active ImGui windows
    for (ImGuiWindow* w : g->Windows) {
        if (!w->Active || w->Hidden) continue;
        if (w->Flags & ImGuiWindowFlags_NoInputs) continue;
        
        ImRect r = w->Rect();
        r.Expand(4.0f);
        if (r.Contains(pt)) {
            return true;
        }
    }
    
    return false;
}

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_vm = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeInit(JNIEnv* env, jobject thiz, jobject asset_mgr, jstring storage_path) {
    LOGI("nativeInit started");
    AAssetManager* mgr = AAssetManager_fromJava(env, asset_mgr);
    AssetManager::get().init(mgr);
    
    if (storage_path) {
        const char* path_str = env->GetStringUTFChars(storage_path, nullptr);
        if (path_str) {
            g_storage_path = path_str;
            EditorUI::get().set_storage_directory(g_storage_path);
            CrashHandlerNative::init("com.textdraw.editor", g_storage_path);
            env->ReleaseStringUTFChars(storage_path, path_str);
        }
    }
    
    // Add default sample textdraws so canvas is never blank
    g_manager.create_box(160.0f, 180.0f, 320.0f, 120.0f, 0x000000B0);
    g_manager.create_text(200.0f, 195.0f, "SAN ANDREAS");
    g_manager.create_sprite(380.0f, 190.0f, 64.0f, 64.0f, "ld_beat:chit");
    g_manager.create_preview_model(210.0f, 220.0f, 75.0f, 60.0f, 411); // Infernus
    
    LOGI("nativeInit completed with sample textdraws and storage: %s", g_storage_path.c_str());
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeSurfaceCreated(JNIEnv* env, jobject thiz) {
    LOGI("nativeSurfaceCreated");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // Avoid writing imgui.ini on Android root
    
    // Connect Android system clipboard to ImGui
    io.SetClipboardTextFn = [](void*, const char* text) {
        android_set_clipboard(text);
    };
    io.GetClipboardTextFn = [](void*) -> const char* {
        static std::string s_clipboard_cache;
        s_clipboard_cache = android_get_clipboard();
        return s_clipboard_cache.c_str();
    };
    
    // Initialize OpenGL textures and systems on active EGL context
    AssetManager::get().init_gl();
    DffRenderer::get().init();
    EditorUI::get().init();
    
    ImGui_ImplOpenGL3_Init("#version 300 es");
    LOGI("ImGui initialized with GLES3 backend and Android clipboard");
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeSurfaceChanged(JNIEnv* env, jobject thiz, jint width, jint height, jfloat density) {
    glViewport(0, 0, width, height);
    g_viewport.set_screen_size(width, height);
    
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    float ui_scale = std::max(1.3f, density * 0.75f);
    EditorUI::get().apply_ui_scale(ui_scale);
    LOGI("Surface changed: %dx%d, density=%.2f, applied ui_scale=%.2f", width, height, density, ui_scale);
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeDrawFrame(JNIEnv* env, jobject thiz) {
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    DffRenderer::get().begin_frame();
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 60.0f;
    ImGui::NewFrame();
    
    // Check if soft keyboard should be shown or hidden
    static bool s_last_want_text = false;
    bool cur_want_text = io.WantTextInput;
    if (cur_want_text != s_last_want_text) {
        s_last_want_text = cur_want_text;
        android_set_keyboard_visible(cur_want_text);
    }
    
    // Render Complete Editor UI & Canvas
    EditorUI::get().render(g_manager, g_viewport);
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeTouchEvent(JNIEnv* env, jobject thiz, jint action, jfloat x, jfloat y, jint pointer_count) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(x, y);
    
    // action: 0=DOWN, 1=UP, 2=MOVE
    if (action == 0) { // ACTION_DOWN
        io.AddMouseButtonEvent(0, true);
        g_last_touch_x = x;
        g_last_touch_y = y;
        
        // If touching UI, do not interact with canvas or clear selection!
        if (is_touch_over_ui(x, y)) {
            g_is_touching_ui = true;
            g_is_dragging_textdraw = false;
            g_is_panning = false;
            return;
        }
        
        g_is_touching_ui = false;
        
        float samp_x, samp_y;
        g_viewport.screen_to_samp(x, y, samp_x, samp_y);
        
        TextDraw* hit = g_manager.hit_test(samp_x, samp_y);
        if (hit) {
            g_manager.save_undo_state();
            if (!g_manager.is_selected(hit->id)) {
                g_manager.select_single(hit->id);
            }
            g_is_dragging_textdraw = true;
            g_is_panning = false;
        } else {
            g_manager.clear_selection();
            g_is_dragging_textdraw = false;
            g_is_panning = (pointer_count >= 2);
        }
    } else if (action == 1) { // ACTION_UP
        io.AddMouseButtonEvent(0, false);
        g_is_touching_ui = false;
        g_is_dragging_textdraw = false;
        g_is_panning = false;
    } else if (action == 2) { // ACTION_MOVE
        if (g_is_touching_ui) {
            g_last_touch_x = x;
            g_last_touch_y = y;
            return;
        }
        
        float dx = x - g_last_touch_x;
        float dy = y - g_last_touch_y;
        
        if (g_is_dragging_textdraw) {
            // Convert screen delta to SA-MP coordinate delta
            float samp_dx = (dx / g_viewport.canvas_screen_w) * 640.0f;
            float samp_dy = (dy / g_viewport.canvas_screen_h) * 480.0f;
            g_manager.move_selected(samp_dx, samp_dy, g_viewport.grid_step);
        } else if (g_is_panning || pointer_count >= 2) {
            g_viewport.add_pan(dx, dy);
        }
        
        g_last_touch_x = x;
        g_last_touch_y = y;
    }
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativePinchZoom(JNIEnv* env, jobject thiz, jfloat scale_factor) {
    float cur = g_viewport.get_zoom();
    g_viewport.set_zoom(cur * scale_factor);
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeInputCharacters(JNIEnv* env, jobject thiz, jstring text) {
    if (!text) return;
    const char* utf = env->GetStringUTFChars(text, nullptr);
    if (utf) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharactersUTF8(utf);
        env->ReleaseStringUTFChars(text, utf);
    }
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeInputKey(JNIEnv* env, jobject thiz, jint keycode) {
    ImGuiIO& io = ImGui::GetIO();
    if (keycode == 67) { // Android KEYCODE_DEL (Backspace)
        io.AddKeyEvent(ImGuiKey_Backspace, true);
        io.AddKeyEvent(ImGuiKey_Backspace, false);
    }
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeSetDialogText(JNIEnv* env, jobject thiz, jint field_id, jstring text) {
    if (!text) return;
    const char* utf = env->GetStringUTFChars(text, nullptr);
    if (utf) {
        EditorUI::get().set_dialog_text(field_id, utf, g_manager);
        env->ReleaseStringUTFChars(text, utf);
    }
}

JNIEXPORT jstring JNICALL
Java_com_textdraw_editor_NativeBridge_nativeExportPawn(JNIEnv* env, jobject thiz) {
    std::string code = PawnExporter::export_pawn(g_manager.get_all_textdraws());
    return env->NewStringUTF(code.c_str());
}

JNIEXPORT void JNICALL
Java_com_textdraw_editor_NativeBridge_nativeImportPawn(JNIEnv* env, jobject thiz, jstring code_str) {
    const char* native_str = env->GetStringUTFChars(code_str, nullptr);
    if (native_str) {
        EditorUI::get().set_import_code(native_str);
        env->ReleaseStringUTFChars(code_str, native_str);
    }
}

} // extern "C"

void EditorUI::set_import_code(const std::string& code) {
    strncpy(import_buffer, code.c_str(), sizeof(import_buffer) - 1);
    open_import_modal();
}
