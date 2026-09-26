#pragma once

#include "TextDrawManager.h"
#include "Viewport.h"
#include "../imgui/imgui.h"
#include <string>

class EditorUI {
public:
    static EditorUI& get();
    
    void init();
    void render(TextDrawManager& manager, Viewport& viewport);
    
    // External interaction triggers
    void open_export_modal();
    void open_import_modal();
    void open_save_project_modal();
    void open_load_project_modal();
    void set_storage_directory(const std::string& path) { storage_path = path; }
    std::string get_exported_code() const { return export_code_buffer; }
    void set_import_code(const std::string& code);
    void set_dialog_text(int field_id, const std::string& text, TextDrawManager& manager);

    void apply_ui_scale(float scale);
    float get_ui_scale() const { return ui_scale; }

private:
    EditorUI() = default;
    float ui_scale = 1.0f;
    
    void apply_gtasa_theme();
    void render_top_bar(TextDrawManager& manager, Viewport& viewport);
    void render_canvas_overlay(TextDrawManager& manager, Viewport& viewport, ImDrawList* draw_list);
    void render_inspector(TextDrawManager& manager, Viewport& viewport);
    void render_bottom_toolbar(TextDrawManager& manager, Viewport& viewport);
    void render_dpad_widget(TextDrawManager& manager, Viewport& viewport);
    
    // Modals & Panels
    void render_file_modal(TextDrawManager& manager);
    void render_edit_modal(TextDrawManager& manager);
    void render_view_modal(Viewport& viewport);
    void render_layers_panel(TextDrawManager& manager);
    void render_group_modal(TextDrawManager& manager);
    void render_trash_modal(TextDrawManager& manager);
    void render_sprite_picker(TextDrawManager& manager);
    void render_model_picker(TextDrawManager& manager);
    void render_export_modal(TextDrawManager& manager);
    void render_import_modal(TextDrawManager& manager);
    void render_save_project_modal(TextDrawManager& manager);
    void render_load_project_modal(TextDrawManager& manager);
    void render_carcols_palette_modal(TextDraw* td, bool target_is_box, int veh_col_index = -1);
    
    bool show_file_modal = false;
    bool show_edit_modal = false;
    bool show_view_modal = false;
    bool show_layers_panel = false;
    bool show_group_modal = false;
    bool show_trash_modal = false;
    bool show_new_project_confirm = false;
    bool show_sprite_picker = false;
    bool show_model_picker = false;
    bool show_export_modal = false;
    bool show_import_modal = false;
    bool show_save_project_modal = false;
    bool show_load_project_modal = false;
    bool show_carcols_picker = false;
    
    char sprite_search_filter[64] = "";
    char model_search_filter[64] = "";
    char layer_search_filter[64] = "";
    char import_buffer[16384] = "";
    char project_name_buf[64] = "my_textdraw_project";
    std::string storage_path = "";
    std::string export_code_buffer;
    
    bool export_only_selected = false;
    bool export_wrap_functions = true;
    
    bool dpad_expanded = true;
    float dpad_step = 1.0f;
    
    // Color picker context
    bool color_target_box = false;
    int color_target_veh_col = -1; // -1 = text/box, 1 = veh col1, 2 = veh col2
};
