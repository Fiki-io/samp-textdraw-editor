#include "TextDrawManager.h"
#include "SampFontRenderer.h"
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
    td.letter_width = 0.30f;
    td.letter_height = 1.50f;
    // text_width/height = 0 means no TextDrawTextSize call (auto)
    td.text_width = 0.0f;
    td.text_height = 0.0f;
    td.color = 0xFFFFFFFF; // White
    td.shadow = 1;
    td.outline = 1;
    td.background_color = 0x00000096; // Black 150-alpha (matches SA-MP default)
    td.use_box = false;
    td.proportional = true;
    
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
    td.letter_height = 0.0f;
    // For LEFT alignment, TextDrawTextSize = absolute bottom-right corner
    td.text_width = x + w;  // right edge in SA-MP coords
    td.text_height = y + h; // bottom edge in SA-MP coords
    td.use_box = true;
    td.box_color = color;
    td.color = 0x00000000;
    td.shadow = 0;
    td.outline = 0;
    td.background_color = 0x000000FF;
    td.proportional = true;
    
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

void TextDrawManager::toggle_selection(int id) {
    for (auto& td : textdraws) {
        if (td.id == id) {
            td.is_selected = !td.is_selected;
            if (td.is_selected) {
                active_id = id;
            } else if (active_id == id) {
                active_id = -1;
                for (auto& other : textdraws) {
                    if (other.is_selected) {
                        active_id = other.id;
                        break;
                    }
                }
            }
            break;
        }
    }
}

void TextDrawManager::select_all() {
    for (auto& td : textdraws) {
        td.is_selected = true;
    }
    if (!textdraws.empty()) active_id = textdraws.back().id;
}

void TextDrawManager::clear_selection() {
    active_id = -1;
    for (auto& td : textdraws) {
        td.is_selected = false;
    }
}

void TextDrawManager::invert_selection() {
    active_id = -1;
    for (auto& td : textdraws) {
        td.is_selected = !td.is_selected;
        if (td.is_selected) active_id = td.id;
    }
}

int TextDrawManager::get_selected_count() const {
    int count = 0;
    for (const auto& td : textdraws) {
        if (td.is_selected) count++;
    }
    return count;
}

bool TextDrawManager::is_selected(int id) const {
    for (const auto& td : textdraws) {
        if (td.id == id) return td.is_selected;
    }
    return false;
}

TextDraw* TextDrawManager::get_textdraw_by_id(int id) {
    for (auto& td : textdraws) {
        if (td.id == id) return &td;
    }
    return nullptr;
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
    time_t now = time(nullptr);
    char time_buf[32] = "12:00:00";
    tm* ltm = localtime(&now);
    if (ltm) strftime(time_buf, sizeof(time_buf), "%H:%M:%S", ltm);
    
    for (const auto& td : textdraws) {
        if (td.is_selected && !td.is_locked) {
            trash_bin.push_back({td, std::string(time_buf)});
            if (trash_bin.size() > 60) {
                trash_bin.erase(trash_bin.begin());
            }
        }
    }
    
    textdraws.erase(
        std::remove_if(textdraws.begin(), textdraws.end(), [](const TextDraw& td) {
            return td.is_selected && !td.is_locked;
        }),
        textdraws.end()
    );
    active_id = -1;
    update_z_indices();
}

void TextDrawManager::set_selected_group(int group_id) {
    save_undo_state();
    for (auto& td : textdraws) {
        if (td.is_selected) {
            td.group_id = group_id;
            td.is_grouped = (group_id > 0);
        }
    }
}

void TextDrawManager::select_by_group(int group_id) {
    active_id = -1;
    for (auto& td : textdraws) {
        td.is_selected = (td.group_id == group_id && group_id > 0);
        if (td.is_selected) active_id = td.id;
    }
}

void TextDrawManager::set_selected_color(uint32_t color, int color_target) {
    save_undo_state();
    for (auto& td : textdraws) {
        if (td.is_selected && !td.is_locked) {
            if (color_target == 0) td.color = color;
            else if (color_target == 1) td.box_color = color;
            else if (color_target == 2) td.background_color = color;
        }
    }
}

void TextDrawManager::set_selected_visibility(bool visible) {
    save_undo_state();
    for (auto& td : textdraws) {
        if (td.is_selected) td.is_visible = visible;
    }
}

void TextDrawManager::set_selected_locked(bool locked) {
    save_undo_state();
    for (auto& td : textdraws) {
        if (td.is_selected) td.is_locked = locked;
    }
}

void TextDrawManager::auto_fit_selected_text_size() {
    save_undo_state();
    for (auto& td : textdraws) {
        if (td.is_selected) {
            td.auto_calculate_text_size();
        }
    }
}

void TextDrawManager::align_selected_left() {
    auto selected = get_selected_textdraws();
    if (selected.size() < 2) return;
    save_undo_state();
    float min_x = 9999.0f;
    for (auto* td : selected) if (td->x < min_x) min_x = td->x;
    for (auto* td : selected) if (!td->is_locked) td->x = min_x;
}

void TextDrawManager::align_selected_center_h() {
    auto selected = get_selected_textdraws();
    if (selected.size() < 2) return;
    save_undo_state();
    float sum_x = 0.0f;
    for (auto* td : selected) sum_x += td->x;
    float avg_x = sum_x / (float)selected.size();
    for (auto* td : selected) if (!td->is_locked) td->x = avg_x;
}

void TextDrawManager::align_selected_right() {
    auto selected = get_selected_textdraws();
    if (selected.size() < 2) return;
    save_undo_state();
    float max_x = -9999.0f;
    for (auto* td : selected) if (td->x > max_x) max_x = td->x;
    for (auto* td : selected) if (!td->is_locked) td->x = max_x;
}

void TextDrawManager::align_selected_top() {
    auto selected = get_selected_textdraws();
    if (selected.size() < 2) return;
    save_undo_state();
    float min_y = 9999.0f;
    for (auto* td : selected) if (td->y < min_y) min_y = td->y;
    for (auto* td : selected) if (!td->is_locked) td->y = min_y;
}

void TextDrawManager::align_selected_center_v() {
    auto selected = get_selected_textdraws();
    if (selected.size() < 2) return;
    save_undo_state();
    float sum_y = 0.0f;
    for (auto* td : selected) sum_y += td->y;
    float avg_y = sum_y / (float)selected.size();
    for (auto* td : selected) if (!td->is_locked) td->y = avg_y;
}

void TextDrawManager::align_selected_bottom() {
    auto selected = get_selected_textdraws();
    if (selected.size() < 2) return;
    save_undo_state();
    float max_y = -9999.0f;
    for (auto* td : selected) if (td->y > max_y) max_y = td->y;
    for (auto* td : selected) if (!td->is_locked) td->y = max_y;
}

void TextDrawManager::move_layer(int from_idx, int to_idx) {
    if (from_idx < 0 || from_idx >= (int)textdraws.size()) return;
    if (to_idx < 0 || to_idx >= (int)textdraws.size()) return;
    if (from_idx == to_idx) return;
    save_undo_state();
    TextDraw td = textdraws[from_idx];
    textdraws.erase(textdraws.begin() + from_idx);
    textdraws.insert(textdraws.begin() + to_idx, td);
    update_z_indices();
}

void TextDrawManager::restore_deleted(size_t index) {
    if (index >= trash_bin.size()) return;
    save_undo_state();
    TextDraw td = trash_bin[index].textdraw;
    td.id = next_id++;
    td.is_selected = true;
    trash_bin.erase(trash_bin.begin() + index);
    textdraws.push_back(td);
    active_id = td.id;
    update_z_indices();
}

void TextDrawManager::restore_all_deleted() {
    if (trash_bin.empty()) return;
    save_undo_state();
    for (auto& item : trash_bin) {
        TextDraw td = item.textdraw;
        td.id = next_id++;
        td.is_selected = true;
        textdraws.push_back(td);
        active_id = td.id;
    }
    trash_bin.clear();
    update_z_indices();
}

void TextDrawManager::empty_trash() {
    trash_bin.clear();
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
        item["group_id"] = td.group_id;
        item["is_grouped"] = td.is_grouped;
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
            td.group_id = item.value("group_id", 0);
            td.is_grouped = item.value("is_grouped", false);
            
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

void TextDraw::auto_calculate_text_size() {
    if (font >= 0 && font <= 3) {
        ImVec2 sz = SampFontRenderer::get().measure_text_samp(font, text, letter_width, letter_height, proportional);
        float est_w = std::max(8.0f, sz.x);
        float est_h = std::max(6.0f, sz.y);
        if (alignment == TextDrawAlignment::LEFT) {
            text_width = x + est_w;
            text_height = y + est_h;
        } else if (alignment == TextDrawAlignment::CENTER) {
            text_width = est_w;
            text_height = est_h;
        } else {
            text_width = x - est_w;
            text_height = y;
        }
    }
}

void TextDraw::get_bounds(float& out_x1, float& out_y1, float& out_x2, float& out_y2) const {
    if (font == 4 || font == 5) {
        out_x1 = x;
        out_y1 = y;
        out_x2 = x + text_width;
        out_y2 = y + text_height;
    } else if (use_box && text_width > 0.0f && text_height > 0.0f) {
        if (alignment == TextDrawAlignment::CENTER) {
            float hw = text_width * 0.5f;
            out_x1 = x - hw;
            out_y1 = y;
            out_x2 = x + hw;
            out_y2 = y + text_height;
        } else if (alignment == TextDrawAlignment::RIGHT) {
            out_x1 = text_width;
            out_y1 = y;
            out_x2 = x;
            out_y2 = text_height;
        } else {
            out_x1 = x;
            out_y1 = y;
            out_x2 = text_width;
            out_y2 = text_height;
        }
    } else {
        ImVec2 sz = SampFontRenderer::get().measure_text_samp(font, text, letter_width, letter_height, proportional);
        float w = std::max(4.0f, sz.x);
        float h = std::max(4.0f, sz.y);
        if (alignment == TextDrawAlignment::CENTER) {
            out_x1 = x - w * 0.5f;
            out_x2 = x + w * 0.5f;
        } else if (alignment == TextDrawAlignment::RIGHT) {
            out_x1 = x - w;
            out_x2 = x;
        } else {
            out_x1 = x;
            out_x2 = x + w;
        }
        out_y1 = y;
        out_y2 = y + h;
    }
}

