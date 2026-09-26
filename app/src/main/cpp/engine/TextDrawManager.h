#pragma once

#include "TextDraw.h"
#include <vector>
#include <memory>
#include <string>

struct DeletedItem {
    TextDraw textdraw;
    std::string time_str;
};

class TextDrawManager {
public:
    TextDrawManager();
    
    // Creation
    TextDraw* create_text(float x = 320.0f, float y = 240.0f, const std::string& text = "New Textdraw");
    TextDraw* create_box(float x = 320.0f, float y = 240.0f, float w = 120.0f, float h = 30.0f, uint32_t color = 0x00000080);
    TextDraw* create_sprite(float x = 320.0f, float y = 240.0f, float w = 64.0f, float h = 64.0f, const std::string& txd_sprite = "ld_beat:chit");
    TextDraw* create_preview_model(float x = 320.0f, float y = 240.0f, float w = 80.0f, float h = 80.0f, int model_id = 411);
    
    // Selection & Multi-select (Grouping)
    void select_single(int id);
    void toggle_selection(int id);
    void select_all();
    void clear_selection();
    void invert_selection();
    int get_selected_count() const;
    bool is_selected(int id) const;
    TextDraw* get_active_textdraw();
    const TextDraw* get_active_textdraw() const;
    std::vector<TextDraw*> get_selected_textdraws();
    TextDraw* get_textdraw_by_id(int id);
    
    // Hit Testing (find clicked textdraw on 640x480 canvas)
    TextDraw* hit_test(float samp_x, float samp_y);
    
    // Modification & Group Actions
    void move_selected(float delta_x, float delta_y, float snap_step = 0.0f);
    void duplicate_selected();
    void delete_selected();
    void set_selected_group(int group_id);
    void select_by_group(int group_id);
    void set_selected_color(uint32_t color, int color_target); // 0: text color, 1: box color, 2: background/shadow color
    void set_selected_visibility(bool visible);
    void set_selected_locked(bool locked);
    void auto_fit_selected_text_size();
    
    // Group Alignment tools
    void align_selected_left();
    void align_selected_center_h();
    void align_selected_right();
    void align_selected_top();
    void align_selected_center_v();
    void align_selected_bottom();
    
    // Layer Reordering
    void bring_to_front();
    void send_to_back();
    void move_layer_up();
    void move_layer_down();
    void move_layer(int from_idx, int to_idx);
    
    // Recycle Bin / Trash (Silinenler)
    void restore_deleted(size_t index);
    void restore_all_deleted();
    void empty_trash();
    const std::vector<DeletedItem>& get_trash() const { return trash_bin; }
    size_t get_trash_count() const { return trash_bin.size(); }
    
    // Access
    std::vector<TextDraw>& get_all_textdraws() { return textdraws; }
    const std::vector<TextDraw>& get_all_textdraws() const { return textdraws; }
    void clear_all();
    
    // Undo / Redo
    void save_undo_state();
    void undo();
    void redo();
    bool can_undo() const;
    bool can_redo() const;

    // Project Persistence (.json)
    std::string serialize_project_to_json(const std::string& project_name = "Untitled") const;
    bool deserialize_project_from_json(const std::string& json_str);
    bool save_project_to_file(const std::string& file_path, const std::string& project_name = "Untitled");
    bool load_project_from_file(const std::string& file_path);

private:
    std::vector<TextDraw> textdraws;
    std::vector<DeletedItem> trash_bin;
    int next_id = 1;
    int active_id = -1;
    
    std::vector<std::vector<TextDraw>> undo_stack;
    std::vector<std::vector<TextDraw>> redo_stack;
    const size_t max_history = 40;
    
    void update_z_indices();
};
