#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <android/asset_manager.h>
#include <GLES3/gl3.h>
#include "../utils/json.hpp"

struct VehicleColor {
    int id;
    uint8_t r, g, b;
    std::string hex;
    std::string name;
};

struct VehicleDef {
    int id;
    std::string name;
    std::string type;
    std::string dff;
    std::string txd;
};

struct SkinDef {
    int id;
    std::string name;
    std::string type;
    std::string dff;
    std::string txd;
};

struct SpriteDef {
    std::string name;
    std::string txd;
    std::string full_name; // e.g. "ld_beat:chit"
    int width;
    int height;
    std::string webp_path;
};

struct FontGlyphRegion {
    float u1, v1, u2, v2;
    int width, height;
};

class AssetManager {
public:
    static AssetManager& get();
    
    void init(AAssetManager* mgr);
    
    // Texture Loading & Caching (OpenGL ES)
    GLuint get_texture(const std::string& asset_path);
    GLuint get_sprite_texture(const std::string& txd_sprite);
    
    // Font Textures
    GLuint font1_texture = 0;
    GLuint font2_texture = 0;
    GLuint mouse_texture = 0;
    GLuint sampgui_texture = 0;
    
    // Data collections
    const std::vector<VehicleColor>& get_colors() const { return colors; }
    const std::vector<VehicleDef>& get_vehicles() const { return vehicles; }
    const std::vector<SkinDef>& get_skins() const { return skins; }
    const std::unordered_map<std::string, std::vector<SpriteDef>>& get_sprites() const { return sprites_by_txd; }
    
    // 3D Model Loading (.dff)
    std::vector<uint8_t> load_model_dff(const std::string& dff_name);
    
    // Helper: read text/binary file from Android assets
    std::string read_asset_text(const std::string& path);
    std::vector<uint8_t> read_asset_bytes(const std::string& path);

private:
    AssetManager() = default;
    ~AssetManager();
    
    AAssetManager* asset_manager = nullptr;
    std::unordered_map<std::string, GLuint> texture_cache;
    
    std::vector<VehicleColor> colors;
    std::vector<VehicleDef> vehicles;
    std::vector<SkinDef> skins;
    std::unordered_map<std::string, std::vector<SpriteDef>> sprites_by_txd;
    nlohmann::json fonts_data_json;
    
    void load_json_databases();
};
