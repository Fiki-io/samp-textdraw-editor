#include "TextDrawManager.h"
#include <algorithm>
#include <cmath>

TextDrawManager::TextDrawManager() {
    clear_all();
}

void TextDrawManager::clear_all() {
    textdraws.clear();
    next_id = 1;
    active_id = -1;
    undo_stack.clear();
    redo_stack.clear();
}

void TextDrawManager::update_z_indices() {
    for (size_t i = 0; i < textdraws.size(); ++i) {
        textdraws[i].z_index = (int)i;
    }
}

TextDraw* TextDrawManager::create_text(float x, float y, const std::string& text) {
    save_undo_state();
    TextDraw td;
    td.id = next_id++;
    td.variable_name = "Textdraw" + std::to_string(td.id);
    td.font = 1; // Standard Chalet London
    td.x = x;
    td.y = y;
    td.text = text;
    td.letter_width = 0.45f;
    td.letter_height = 1.6f;
    td.text_width = 120.0f;
    td.text_height = 20.0f;
    td.color = 0xFFFFFFFF; // White
    td.shadow = 1;
    td.outline = 0;
    td.use_box = false;
    
    textdraws.push_back(td);
    update_z_indices();
    select_single(td.id);
    return get_active_textdraw();
}

TextDraw* TextDrawManager::create_box(float x, float y, float w, float h, uint32_t color) {
    save_undo_state();
    TextDraw td;
    td.id = next_id++;
    td.variable_name = "Textdraw" + std::to_string(td.id);
    td.font = 1;
    td.x = x;
    td.y = y;
    td.text = "_"; // SA-MP standard dummy text for box
    td.letter_width = 0.0f;
    td.letter_height = (h / 8.5f); // Scale box height
    td.text_width = w;
    td.text_height = h;
    td.use_box = true;
    td.box_color = color;
    td.color = 0x00000000;
    
    textdraws.push_back(td);
    update_z_indices();
    select_single(td.id);
    return get_active_textdraw();
}

TextDraw* TextDrawManager::create_sprite(float x, float y, float w, float h, const std::string& txd_sprite) {
    save_undo_state();
    TextDraw td;
    td.id = next_id++;
    td.variable_name = "Textdraw" + std::to_string(td.id);
    td.font = 4; // Font 4: Sprite
    td.x = x;
    td.y = y;
    td.text = txd_sprite;
    td.text_width = w;
    td.text_height = h;
    td.color = 0xFFFFFFFF;
    
    textdraws.push_back(td);
    update_z_indices();
    select_single(td.id);
    return get_active_textdraw();
}

TextDraw* TextDrawManager::create_preview_model(float x, float y, float w, float h, int model_id) {
    save_undo_state();
    TextDraw td;
    td.id = next_id++;
    td.variable_name = "Textdraw" + std::to_string(td.id);
    td.font = 5; // Font 5: 3D Model Preview
    td.x = x;
    td.y = y;
    td.text = std::to_string(model_id);
    td.preview_model = model_id;
    td.text_width = w;
    td.text_height = h;
    td.color = 0xFFFFFFFF;
    if (model_id >= 400 && model_id <= 611) {
        td.rot_x = -16.0f;
        td.rot_y = 0.0f;
        td.rot_z = -55.0f;
        td.zoom = 1.0f;
        td.veh_color1 = 3;
        td.veh_color2 = 3;
    } else if (model_id >= 321 && model_id <= 372) {
        td.rot_x = -15.0f;
        td.rot_y = 0.0f;
        td.rot_z = -45.0f;
        td.zoom = 1.2f;
        td.veh_color1 = 1;
        td.veh_color2 = 1;
    } else {
        td.rot_x = 0.0f;
        td.rot_y = 0.0f;
        td.rot_z = -15.0f;
        td.zoom = 0.85f;
        td.veh_color1 = 1;
        td.veh_color2 = 1;
    }
    
    textdraws.push_back(td);
    update_z_indices();
    select_single(td.id);
    return get_active_textdraw();
}

void TextDrawManager::select_single(int id) {
    active_id = id;
    for (auto& td : textdraws) {
        td.is_selected = (td.id == id);
    }
}

void TextDrawManager::clear_selection() {
    active_id = -1;
    for (auto& td : textdraws) {
        td.is_selected = false;
    }
}

TextDraw* TextDrawManager::get_active_textdraw() {
    if (active_id == -1) return nullptr;
    for (auto& td : textdraws) {
        if (td.id == active_id) return &td;
    }
    return nullptr;
}

const TextDraw* TextDrawManager::get_active_textdraw() const {
    if (active_id == -1) return nullptr;
    for (const auto& td : textdraws) {
        if (td.id == active_id) return &td;
    }
    return nullptr;
}

std::vector<TextDraw*> TextDrawManager::get_selected_textdraws() {
    std::vector<TextDraw*> sel;
    for (auto& td : textdraws) {
        if (td.is_selected) sel.push_back(&td);
    }
    return sel;
}

TextDraw* TextDrawManager::hit_test(float samp_x, float samp_y) {
    // Iterate backwards (top layer first)
    for (int i = (int)textdraws.size() - 1; i >= 0; --i) {
        if (!textdraws[i].is_visible || textdraws[i].is_locked) continue;
        float x1, y1, x2, y2;
        textdraws[i].get_bounds(x1, y1, x2, y2);
        
        // Add small padding for easier mobile touch hit testing
        const float pad = 3.0f;
        if (samp_x >= (x1 - pad) && samp_x <= (x2 + pad) &&
            samp_y >= (y1 - pad) && samp_y <= (y2 + pad)) {
            return &textdraws[i];
        }
    }
    return nullptr;
}

void TextDrawManager::move_selected(float delta_x, float delta_y, float snap_step) {
    for (auto& td : textdraws) {
        if (td.is_selected && !td.is_locked) {
            td.x += delta_x;
            td.y += delta_y;
            if (snap_step > 0.001f) {
                td.x = std::round(td.x / snap_step) * snap_step;
                td.y = std::round(td.y / snap_step) * snap_step;
            }
        }
    }
}

void TextDrawManager::duplicate_selected() {
    auto selected = get_selected_textdraws();
    if (selected.empty()) return;
    
    save_undo_state();
    clear_selection();
    
    for (auto* src : selected) {
        TextDraw copy = *src;
        copy.id = next_id++;
        copy.variable_name = "Textdraw" + std::to_string(copy.id);
        copy.x += 10.0f;
        copy.y += 10.0f;
        copy.is_selected = true;
        textdraws.push_back(copy);
        active_id = copy.id;
    }
    update_z_indices();
}

void TextDrawManager::delete_selected() {
    save_undo_state();
    textdraws.erase(
        std::remove_if(textdraws.begin(), textdraws.end(), [](const TextDraw& td) {
            return td.is_selected && !td.is_locked;
        }),
        textdraws.end()
    );
    active_id = -1;
    update_z_indices();
}

void TextDrawManager::bring_to_front() {
    if (active_id == -1) return;
    save_undo_state();
    auto it = std::find_if(textdraws.begin(), textdraws.end(), [this](const TextDraw& td) {
        return td.id == active_id;
    });
    if (it != textdraws.end() && it + 1 != textdraws.end()) {
        TextDraw td = *it;
        textdraws.erase(it);
        textdraws.push_back(td);
        update_z_indices();
    }
}

void TextDrawManager::send_to_back() {
    if (active_id == -1) return;
    save_undo_state();
    auto it = std::find_if(textdraws.begin(), textdraws.end(), [this](const TextDraw& td) {
        return td.id == active_id;
    });
    if (it != textdraws.end() && it != textdraws.begin()) {
        TextDraw td = *it;
        textdraws.erase(it);
        textdraws.insert(textdraws.begin(), td);
        update_z_indices();
    }
}

void TextDrawManager::move_layer_up() {
    if (active_id == -1) return;
    save_undo_state();
    auto it = std::find_if(textdraws.begin(), textdraws.end(), [this](const TextDraw& td) {
        return td.id == active_id;
    });
    if (it != textdraws.end() && it + 1 != textdraws.end()) {
        std::iter_swap(it, it + 1);
        update_z_indices();
    }
}

void TextDrawManager::move_layer_down() {
    if (active_id == -1) return;
    save_undo_state();
    auto it = std::find_if(textdraws.begin(), textdraws.end(), [this](const TextDraw& td) {
        return td.id == active_id;
    });
    if (it != textdraws.end() && it != textdraws.begin()) {
        std::iter_swap(it, it - 1);
        update_z_indices();
    }
}

void TextDrawManager::save_undo_state() {
    undo_stack.push_back(textdraws);
    if (undo_stack.size() > max_history) {
        undo_stack.erase(undo_stack.begin());
    }
    redo_stack.clear();
}

void TextDrawManager::undo() {
    if (undo_stack.empty()) return;
    redo_stack.push_back(textdraws);
    textdraws = undo_stack.back();
    undo_stack.pop_back();
    update_z_indices();
}

void TextDrawManager::redo() {
    if (redo_stack.empty()) return;
    undo_stack.push_back(textdraws);
    textdraws = redo_stack.back();
    redo_stack.pop_back();
    update_z_indices();
}

bool TextDrawManager::can_undo() const {
    return !undo_stack.empty();
}

bool TextDrawManager::can_redo() const {
    return !redo_stack.empty();
}

#include "../utils/json.hpp"
#include <fstream>

std::string TextDrawManager::serialize_project_to_json(const std::string& project_name) const {
    nlohmann::json root;
    root["project_name"] = project_name;
    root["version"] = 1;
    
    nlohmann::json td_array = nlohmann::json::array();
    for (const auto& td : textdraws) {
        nlohmann::json item;
        item["id"] = td.id;
        item["variable_name"] = td.variable_name;
        item["is_player"] = td.is_player;
        item["x"] = td.x;
        item["y"] = td.y;
        item["text"] = td.text;
        item["font"] = td.font;
        item["letter_width"] = td.letter_width;
        item["letter_height"] = td.letter_height;
        item["text_width"] = td.text_width;
        item["text_height"] = td.text_height;
        item["alignment"] = static_cast<int>(td.alignment);
        item["color"] = td.color;
        item["use_box"] = td.use_box;
        item["box_color"] = td.box_color;
        item["shadow"] = td.shadow;
        item["outline"] = td.outline;
        item["background_color"] = td.background_color;
        item["proportional"] = td.proportional;
        item["selectable"] = td.selectable;
        item["preview_model"] = td.preview_model;
        item["rot_x"] = td.rot_x;
        item["rot_y"] = td.rot_y;
        item["rot_z"] = td.rot_z;
        item["zoom"] = td.zoom;
        item["veh_color1"] = td.veh_color1;
        item["veh_color2"] = td.veh_color2;
        td_array.push_back(item);
    }
    root["textdraws"] = td_array;
    return root.dump(2);
}

bool TextDrawManager::deserialize_project_from_json(const std::string& json_str) {
    try {
        auto root = nlohmann::json::parse(json_str);
        if (!root.contains("textdraws") || !root["textdraws"].is_array()) {
            return false;
        }
        
        save_undo_state();
        textdraws.clear();
        next_id = 1;
        active_id = -1;
        
        for (const auto& item : root["textdraws"]) {
            TextDraw td;
            td.id = item.value("id", next_id);
            if (td.id >= next_id) next_id = td.id + 1;
            td.variable_name = item.value("variable_name", "Textdraw" + std::to_string(td.id));
            td.is_player = item.value("is_player", false);
            td.x = item.value("x", 320.0f);
            td.y = item.value("y", 240.0f);
            td.text = item.value("text", "Textdraw");
            td.font = item.value("font", 1);
            td.letter_width = item.value("letter_width", 0.45f);
            td.letter_height = item.value("letter_height", 1.6f);
            td.text_width = item.value("text_width", 120.0f);
            td.text_height = item.value("text_height", 20.0f);
            td.alignment = static_cast<TextDrawAlignment>(item.value("alignment", 1));
            td.color = item.value("color", 0xFFFFFFFF);
            td.use_box = item.value("use_box", false);
            td.box_color = item.value("box_color", 0x00000080);
            td.shadow = item.value("shadow", 1);
            td.outline = item.value("outline", 0);
            td.background_color = item.value("background_color", 0x000000FF);
            td.proportional = item.value("proportional", true);
            td.selectable = item.value("selectable", false);
            td.preview_model = item.value("preview_model", 411);
            td.rot_x = item.value("rot_x", 0.0f);
            td.rot_y = item.value("rot_y", 0.0f);
            td.rot_z = item.value("rot_z", 0.0f);
            td.zoom = item.value("zoom", 1.0f);
            td.veh_color1 = item.value("veh_color1", 1);
            td.veh_color2 = item.value("veh_color2", 1);
            
            textdraws.push_back(td);
        }
        update_z_indices();
        if (!textdraws.empty()) {
            select_single(textdraws.front().id);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool TextDrawManager::save_project_to_file(const std::string& file_path, const std::string& project_name) {
    std::string json_str = serialize_project_to_json(project_name);
    std::ofstream ofs(file_path);
    if (!ofs.is_open()) return false;
    ofs << json_str;
    return true;
}

bool TextDrawManager::load_project_from_file(const std::string& file_path) {
    std::ifstream ifs(file_path);
    if (!ifs.is_open()) return false;
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return deserialize_project_from_json(str);
}

