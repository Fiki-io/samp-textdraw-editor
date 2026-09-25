#include "AssetManager.h"
#include <android/log.h>
#include <EGL/egl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../utils/stb_image.h"

#define LOG_TAG "TextDraw_AssetManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

AssetManager& AssetManager::get() {
    static AssetManager instance;
    return instance;
}

AssetManager::~AssetManager() {
    for (auto& pair : texture_cache) {
        if (pair.second != 0) {
            glDeleteTextures(1, &pair.second);
        }
    }
    texture_cache.clear();
}

void AssetManager::init(AAssetManager* mgr) {
    asset_manager = mgr;
    if (!asset_manager) {
        LOGE("AssetManager initialized with null AAssetManager!");
        return;
    }
    LOGI("AssetManager initialized successfully.");
    
    // Load databases
    load_json_databases();
    LOGI("Databases loaded successfully in nativeInit.");
}

void AssetManager::init_gl() {
    LOGI("AssetManager::init_gl loading core textures with active EGL context...");
    font1_texture = get_texture("fonts/font1.png");
    font2_texture = get_texture("fonts/font2.png");
    mouse_texture = get_texture("ui/mouse.png");
    sampgui_texture = get_texture("ui/sampgui.png");
    LOGI("Preloaded core fonts and UI textures (font1=%u, font2=%u, mouse=%u, sampgui=%u).",
         font1_texture, font2_texture, mouse_texture, sampgui_texture);
}

std::vector<uint8_t> AssetManager::read_asset_bytes(const std::string& path) {
    std::vector<uint8_t> buffer;
    if (!asset_manager) return buffer;
    
    AAsset* asset = AAssetManager_open(asset_manager, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        // Try with leading app_assets or relative
        return buffer;
    }
    
    size_t size = AAsset_getLength(asset);
    buffer.resize(size);
    AAsset_read(asset, buffer.data(), size);
    AAsset_close(asset);
    return buffer;
}

std::string AssetManager::read_asset_text(const std::string& path) {
    auto bytes = read_asset_bytes(path);
    if (bytes.empty()) return "";
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

void AssetManager::load_json_databases() {
    // 1. Carcols
    try {
        std::string carcols_str = read_asset_text("data/carcols.json");
        if (!carcols_str.empty()) {
            auto j = nlohmann::json::parse(carcols_str);
            colors.clear();
            for (const auto& item : j) {
                VehicleColor vc;
                vc.id = item.value("id", 0);
                vc.r = (uint8_t)item.value("r", 255);
                vc.g = (uint8_t)item.value("g", 255);
                vc.b = (uint8_t)item.value("b", 255);
                vc.hex = item.value("hex", "#FFFFFF");
                vc.name = item.value("name", "Unknown");
                colors.push_back(vc);
            }
            LOGI("Loaded %zu vehicle colors from carcols.json", colors.size());
        }
    } catch (const std::exception& e) {
        LOGE("Failed to load carcols.json: %s", e.what());
    }
    
    // 2. Vehicles
    try {
        std::string veh_str = read_asset_text("data/vehicles.json");
        if (!veh_str.empty()) {
            auto j = nlohmann::json::parse(veh_str);
            vehicles.clear();
            for (const auto& item : j) {
                VehicleDef vd;
                vd.id = item.value("id", 400);
                vd.name = item.value("name", "Vehicle");
                vd.type = item.value("type", "Automobile");
                vd.dff = item.value("dff", "");
                vd.txd = item.value("txd", "");
                vehicles.push_back(vd);
            }
            LOGI("Loaded %zu vehicles from vehicles.json", vehicles.size());
        }
    } catch (const std::exception& e) {
        LOGE("Failed to load vehicles.json: %s", e.what());
    }
    
    // 3. Skins
    try {
        std::string skin_str = read_asset_text("data/skins.json");
        if (!skin_str.empty()) {
            auto j = nlohmann::json::parse(skin_str);
            skins.clear();
            for (const auto& item : j) {
                SkinDef sd;
                sd.id = item.value("id", 0);
                sd.name = item.value("name", "Skin");
                sd.type = item.value("type", "Ped");
                sd.dff = item.value("dff", "");
                sd.txd = item.value("txd", "");
                skins.push_back(sd);
            }
            LOGI("Loaded %zu skins from skins.json", skins.size());
        }
    } catch (const std::exception& e) {
        LOGE("Failed to load skins.json: %s", e.what());
    }
    
    // 4. Sprites Manifest
    try {
        std::string sprite_str = read_asset_text("data/sprites_manifest.json");
        if (!sprite_str.empty()) {
            auto j = nlohmann::json::parse(sprite_str);
            size_t count = 0;
            sprites_by_txd.clear();
            for (auto& [txd, list] : j.items()) {
                std::vector<SpriteDef> sdefs;
                for (const auto& item : list) {
                    SpriteDef sd;
                    sd.name = item.value("name", "");
                    sd.txd = item.value("txd", txd);
                    sd.full_name = item.value("full_txd_sprite", txd + ":" + sd.name);
                    sd.width = item.value("width", 64);
                    sd.height = item.value("height", 64);
                    sd.webp_path = item.value("png_path", "sprites/" + txd + "/" + sd.name + ".png");
                    sdefs.push_back(sd);
                    count++;
                }
                sprites_by_txd[txd] = sdefs;
            }
            LOGI("Loaded %zu sprites across %zu TXD groups", count, sprites_by_txd.size());
        }
    } catch (const std::exception& e) {
        LOGE("Failed to load sprites_manifest.json: %s", e.what());
    }
}

GLuint AssetManager::get_texture(const std::string& asset_path) {
    if (asset_path.empty()) return 0;

    EGLContext egl_ctx = eglGetCurrentContext();
    if (egl_ctx == EGL_NO_CONTEXT) {
        LOGE("get_texture called without active EGL context for: %s", asset_path.c_str());
        return 0;
    }
    
    auto it = texture_cache.find(asset_path);
    if (it != texture_cache.end()) {
        return it->second;
    }
    
    auto bytes = read_asset_bytes(asset_path);
    if (bytes.empty()) {
        LOGE("Could not read asset for texture: %s", asset_path.c_str());
        return 0;
    }
    
    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load_from_memory(bytes.data(), (int)bytes.size(), &w, &h, &channels, 4);
    if (!data) {
        LOGE("stbi_load_from_memory failed for %s", asset_path.c_str());
        return 0;
    }
    
    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    stbi_image_free(data);
    
    texture_cache[asset_path] = tex_id;
    return tex_id;
}

GLuint AssetManager::get_sprite_texture(const std::string& txd_sprite) {
    size_t colon = txd_sprite.find(':');
    if (colon == std::string::npos) return 0;
    
    std::string txd = txd_sprite.substr(0, colon);
    std::string name = txd_sprite.substr(colon + 1);
    
    std::transform(txd.begin(), txd.end(), txd.begin(), ::tolower);
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    
    std::string path = "sprites/" + txd + "/" + name + ".png";
    return get_texture(path);
}

std::vector<uint8_t> AssetManager::load_model_dff(const std::string& dff_name) {
    std::string path = "models_3d/" + dff_name;
    return read_asset_bytes(path);
}
