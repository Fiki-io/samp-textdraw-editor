#include "EditorUI.h"
#include "../engine/AssetManager.h"
#include "../engine/DffRenderer.h"
#include "../engine/PawnExporter.h"
#include "../utils/SampColorParser.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <dirent.h>

EditorUI& EditorUI::get() {
    static EditorUI instance;
    return instance;
}

extern void android_show_text_dialog(const char* title, const char* initial_text, int field_id);
extern std::string android_get_clipboard();

void EditorUI::init() {
    apply_gtasa_theme();
}

void EditorUI::apply_ui_scale(float scale) {
    ui_scale = std::clamp(scale, 1.15f, 1.40f);
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = ui_scale;
    apply_gtasa_theme();
}

void EditorUI::apply_gtasa_theme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    // GTA SA Dark & Gold Theme
    colors[ImGuiCol_Text]                  = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.10f, 0.12f, 0.94f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.12f, 0.12f, 0.14f, 0.70f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.12f, 0.12f, 0.15f, 0.98f);
    colors[ImGuiCol_Border]                = ImVec4(0.24f, 0.24f, 0.28f, 0.60f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.16f, 0.20f, 0.85f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.24f, 0.24f, 0.30f, 0.85f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.30f, 0.26f, 0.18f, 0.90f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.18f, 0.14f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.08f, 0.08f, 0.10f, 0.75f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.08f, 0.10f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.25f, 0.25f, 0.30f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.40f, 0.40f, 0.45f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.85f, 0.65f, 0.15f, 0.90f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.95f, 0.72f, 0.15f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.85f, 0.65f, 0.15f, 0.90f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(1.00f, 0.80f, 0.20f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.20f, 0.20f, 0.25f, 0.85f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.35f, 0.30f, 0.18f, 0.90f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.85f, 0.65f, 0.15f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.24f, 0.20f, 0.12f, 0.80f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.40f, 0.32f, 0.15f, 0.80f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.85f, 0.65f, 0.15f, 0.90f);
    colors[ImGuiCol_Separator]             = ImVec4(0.25f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.85f, 0.65f, 0.15f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.85f, 0.65f, 0.15f, 0.70f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 0.80f, 0.20f, 1.00f);
    colors[ImGuiCol_Tab]                   = ImVec4(0.15f, 0.15f, 0.18f, 0.85f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.40f, 0.32f, 0.15f, 0.85f);
    colors[ImGuiCol_TabActive]             = ImVec4(0.28f, 0.22f, 0.10f, 1.00f);

    float s = ui_scale;
    style.WindowRounding    = 8.0f * s;
    style.ChildRounding     = 6.0f * s;
    style.FrameRounding     = 6.0f * s;
    style.PopupRounding     = 8.0f * s;
    style.ScrollbarRounding = 6.0f * s;
    style.GrabRounding      = 4.0f * s;
    style.TabRounding       = 6.0f * s;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.5f;
    style.ItemSpacing       = ImVec2(8.0f * s, 6.0f * s);
    style.TouchExtraPadding = ImVec2(6.0f * s, 6.0f * s);
    style.FramePadding      = ImVec2(8.0f * s, 6.0f * s);
    style.WindowPadding     = ImVec2(10.0f * s, 10.0f * s);
}

void EditorUI::render(TextDrawManager& manager, Viewport& viewport) {
    ImGuiIO& io = ImGui::GetIO();
    viewport.set_screen_size((int)io.DisplaySize.x, (int)io.DisplaySize.y);
    
    // Background canvas pass
    ImDrawList* bg_draw_list = ImGui::GetBackgroundDrawList();
    render_canvas_overlay(manager, viewport, bg_draw_list);
    
    // UI Chrome
    render_top_bar(manager, viewport);
    render_inspector(manager, viewport);
    render_bottom_toolbar(manager, viewport);
    render_dpad_widget(manager, viewport);
    
    // Modals
    if (show_file_modal) render_file_modal(manager);
    if (show_edit_modal) render_edit_modal(manager);
    if (show_view_modal) render_view_modal(viewport);
    if (show_sprite_picker) render_sprite_picker(manager);
    if (show_model_picker) render_model_picker(manager);
    if (show_export_modal) render_export_modal(manager);
    if (show_import_modal) render_import_modal(manager);
    if (show_save_project_modal) render_save_project_modal(manager);
    if (show_load_project_modal) render_load_project_modal(manager);
    if (show_carcols_picker) render_carcols_palette_modal(manager.get_active_textdraw(), color_target_box, color_target_veh_col);
}

void EditorUI::render_canvas_overlay(TextDrawManager& manager, Viewport& viewport, ImDrawList* draw_list) {
    // 1. Draw Virtual Canvas Frame (640x480 boundary)
    float c_x1 = viewport.canvas_screen_x;
    float c_y1 = viewport.canvas_screen_y;
    float c_x2 = c_x1 + viewport.canvas_screen_w;
    float c_y2 = c_y1 + viewport.canvas_screen_h;
    
    // Canvas background
    draw_list->AddRectFilled(ImVec2(c_x1, c_y1), ImVec2(c_x2, c_y2), IM_COL32(20, 22, 26, 255));
    
    // Canvas grid lines if enabled
    if (viewport.enable_grid && viewport.zoom >= 0.8f) {
        float step = std::max(viewport.grid_step, 10.0f);
        ImU32 grid_col = IM_COL32(255, 255, 255, 12);
        for (float gx = 0.0f; gx <= 640.0f; gx += step) {
            float sx, sy1, sy2;
            viewport.samp_to_screen(gx, 0.0f, sx, sy1);
            viewport.samp_to_screen(gx, 480.0f, sx, sy2);
            draw_list->AddLine(ImVec2(sx, sy1), ImVec2(sx, sy2), grid_col);
        }
        for (float gy = 0.0f; gy <= 480.0f; gy += step) {
            float sx1, sx2, sy;
            viewport.samp_to_screen(0.0f, gy, sx1, sy);
            viewport.samp_to_screen(640.0f, gy, sx2, sy);
            draw_list->AddLine(ImVec2(sx1, sy), ImVec2(sx2, sy), grid_col);
        }
    }
    
    // Border around 640x480
    draw_list->AddRect(ImVec2(c_x1, c_y1), ImVec2(c_x2, c_y2), IM_COL32(241, 168, 10, 180), 0.0f, 0, 1.5f);
    
    // 2. Render all TextDraws in Z-order
    const auto& list = manager.get_all_textdraws();
    for (const auto& td : list) {
        if (!td.is_visible) continue;
        
        float sx, sy;
        viewport.samp_to_screen(td.x, td.y, sx, sy);
        float sw = viewport.samp_to_screen_scale_x(td.text_width);
        float sh = viewport.samp_to_screen_scale_y(td.text_height);
        
        // A. Draw Box if enabled
        if (td.use_box) {
            // Box Color in RGBA -> ABGR for ImGui
            uint32_t c = td.box_color;
            ImU32 im_col = IM_COL32((c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
            draw_list->AddRectFilled(ImVec2(sx, sy), ImVec2(sx + sw, sy + sh), im_col);
        }
        
        // B. Draw Content based on Font
        if (td.font == 4) { // Sprite
            GLuint tex = AssetManager::get().get_sprite_texture(td.text);
            uint32_t c = td.color;
            ImU32 im_tint = IM_COL32((c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
            if (tex != 0) {
                draw_list->AddImage((ImTextureID)(intptr_t)tex, ImVec2(sx, sy), ImVec2(sx + sw, sy + sh),
                                   ImVec2(0, 0), ImVec2(1, 1), im_tint);
            } else {
                // Placeholder preview
                draw_list->AddRect(ImVec2(sx, sy), ImVec2(sx + sw, sy + sh), IM_COL32(255, 100, 100, 200));
                draw_list->AddText(ImVec2(sx + 4, sy + 4), IM_COL32(255, 200, 200, 255), td.text.c_str());
            }
        } else if (td.font == 5) { // 3D Model Preview
            GLuint tex = DffRenderer::get().render_to_texture(
                td.preview_model, td.rot_x, td.rot_y, td.rot_z, td.zoom,
                td.veh_color1, td.veh_color2
            );
            if (tex != 0) {
                // UVs flipped vertically for OpenGL FBO texture
                draw_list->AddImage((ImTextureID)(intptr_t)tex,
                                   ImVec2(sx, sy), ImVec2(sx + sw, sy + sh),
                                   ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            } else {
                draw_list->AddRect(ImVec2(sx, sy), ImVec2(sx + sw, sy + sh), IM_COL32(255, 180, 50, 200));
                draw_list->AddText(ImVec2(sx + 4, sy + 4), IM_COL32(255, 255, 255, 255), "3D Model");
            }
        } else { // Fonts 0, 1, 2, 3 (Text)
            float font_scale = (td.letter_height * 0.9f) * (viewport.canvas_screen_h / 480.0f);
            float font_size = 16.0f * font_scale;
            uint32_t bg_c = td.background_color;
            ImU32 im_bg = IM_COL32((bg_c >> 24) & 0xFF, (bg_c >> 16) & 0xFF, (bg_c >> 8) & 0xFF, bg_c & 0xFF);
            
            // Parse SA-MP formatting tags (~r~, ~g~, ~b~, ~w~, ~y~, ~n~)
            auto spans = SampColorParser::parse(td.text, td.color);
            
            float cur_x = sx;
            float cur_y = sy;
            float line_height = font_size * 1.15f;
            
            for (const auto& span : spans) {
                if (span.is_newline) {
                    cur_x = sx;
                    cur_y += line_height;
                    continue;
                }
                if (span.text.empty()) continue;
                
                uint32_t c = span.color;
                ImU32 im_col = IM_COL32((c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
                
                // Shadow
                if (td.shadow > 0) {
                    float sh_off = (float)td.shadow * 1.5f;
                    draw_list->AddText(nullptr, font_size, ImVec2(cur_x + sh_off, cur_y + sh_off), im_bg, span.text.c_str());
                }
                // Outline
                if (td.outline > 0) {
                    float o = (float)td.outline;
                    draw_list->AddText(nullptr, font_size, ImVec2(cur_x - o, cur_y), im_bg, span.text.c_str());
                    draw_list->AddText(nullptr, font_size, ImVec2(cur_x + o, cur_y), im_bg, span.text.c_str());
                    draw_list->AddText(nullptr, font_size, ImVec2(cur_x, cur_y - o), im_bg, span.text.c_str());
                    draw_list->AddText(nullptr, font_size, ImVec2(cur_x, cur_y + o), im_bg, span.text.c_str());
                }
                
                draw_list->AddText(nullptr, font_size, ImVec2(cur_x, cur_y), im_col, span.text.c_str());
                
                // Advance cursor X
                ImVec2 span_size = ImGui::GetFont()->CalcTextSizeA(font_size, FLT_MAX, 0.0f, span.text.c_str());
                cur_x += span_size.x;
            }
        }
        
        // C. Selection Bounding Box & Handles
        if (td.is_selected) {
            float x1, y1, x2, y2;
            td.get_bounds(x1, y1, x2, y2);
            float b_sx1, b_sy1, b_sx2, b_sy2;
            viewport.samp_to_screen(x1, y1, b_sx1, b_sy1);
            viewport.samp_to_screen(x2, y2, b_sx2, b_sy2);
            
            // Neon cyan border
            draw_list->AddRect(ImVec2(b_sx1, b_sy1), ImVec2(b_sx2, b_sy2), IM_COL32(0, 230, 255, 255), 2.0f, 0, 1.5f);
            
            // Corner handles
            const float h_size = 5.0f;
            draw_list->AddRectFilled(ImVec2(b_sx1 - h_size, b_sy1 - h_size), ImVec2(b_sx1 + h_size, b_sy1 + h_size), IM_COL32(0, 230, 255, 255));
            draw_list->AddRectFilled(ImVec2(b_sx2 - h_size, b_sy1 - h_size), ImVec2(b_sx2 + h_size, b_sy1 + h_size), IM_COL32(0, 230, 255, 255));
            draw_list->AddRectFilled(ImVec2(b_sx1 - h_size, b_sy2 - h_size), ImVec2(b_sx1 + h_size, b_sy2 + h_size), IM_COL32(0, 230, 255, 255));
            draw_list->AddRectFilled(ImVec2(b_sx2 - h_size, b_sy2 - h_size), ImVec2(b_sx2 + h_size, b_sy2 + h_size), IM_COL32(0, 230, 255, 255));
            
            // Coordinate tag
            char tag[64];
            snprintf(tag, sizeof(tag), "X:%.1f Y:%.1f", td.x, td.y);
            draw_list->AddText(ImVec2(b_sx1, b_sy1 - 16.0f), IM_COL32(0, 230, 255, 255), tag);
        }
    }
}

void EditorUI::render_top_bar(TextDrawManager& manager, Viewport& viewport) {
    if (ImGui::BeginMainMenuBar()) {
        ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.15f, 1.0f), "GTA SA-MP");
        ImGui::Separator();
        
        // Touch-safe top buttons (no auto-dropdown that triggers accidental touch clicks)
        if (ImGui::Button("FILE")) {
            show_file_modal = true;
        }
        if (ImGui::Button("EDIT")) {
            show_edit_modal = true;
        }
        if (ImGui::Button("VIEW")) {
            show_view_modal = true;
        }
        
        ImGui::Separator();
        if (ImGui::Button("Undo") && manager.can_undo()) manager.undo();
        if (ImGui::Button("Redo") && manager.can_redo()) manager.redo();
        
        ImGui::Separator();
        ImGui::Text("Snap:");
        ImGui::SetNextItemWidth(65.0f * ui_scale);
        float steps[] = {0.1f, 0.5f, 1.0f, 5.0f, 10.0f};
        const char* step_names[] = {"0.1", "0.5", "1.0", "5.0", "10.0"};
        int cur_step = 2; // default 1.0
        for (int i = 0; i < 5; ++i) if (std::abs(viewport.grid_step - steps[i]) < 0.05f) cur_step = i;
        if (ImGui::Combo("##Snap", &cur_step, step_names, 5)) {
            viewport.grid_step = steps[cur_step];
            dpad_step = steps[cur_step];
        }
        
        // Export button right aligned
        float exp_btn_w = 110.0f * ui_scale;
        if (ImGui::GetWindowWidth() > 480.0f) {
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - exp_btn_w - 10.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.65f, 0.15f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
            if (ImGui::Button("EXPORT PWN", ImVec2(exp_btn_w, 0))) {
                open_export_modal();
            }
            ImGui::PopStyleColor(2);
        }
        
        ImGui::EndMainMenuBar();
    }
}

void EditorUI::render_file_modal(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.88f, 380.0f * s);
    float h = std::min(io.DisplaySize.y * 0.88f, 340.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("File Menu##Modal", &show_file_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImVec2 b_size(-1, 38.0f * s);
        
        if (show_new_project_confirm) {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Konfirmasi Buat Project Baru?");
            ImGui::TextWrapped("Semua TextDraw saat ini akan dihapus.");
            ImGui::Spacing();
            if (ImGui::Button("YA, BERSIHKAN SEMUA", ImVec2(-1, 40.0f * s))) {
                manager.clear_all();
                show_new_project_confirm = false;
                show_file_modal = false;
            }
            if (ImGui::Button("BATAL", ImVec2(-1, 34.0f * s))) {
                show_new_project_confirm = false;
            }
        } else {
            if (ImGui::Button("+ New Project", b_size)) {
                if (manager.get_all_textdraws().empty()) {
                    manager.clear_all();
                    show_file_modal = false;
                } else {
                    show_new_project_confirm = true;
                }
            }
            if (ImGui::Button("Save Project (.json)", b_size)) {
                show_file_modal = false;
                open_save_project_modal();
            }
            if (ImGui::Button("Open Project (.json)", b_size)) {
                show_file_modal = false;
                open_load_project_modal();
            }
            if (ImGui::Button("Export Pawn Code (.pwn)", b_size)) {
                show_file_modal = false;
                open_export_modal();
            }
            if (ImGui::Button("Import Pawn Code...", b_size)) {
                show_file_modal = false;
                open_import_modal();
            }
            ImGui::Spacing();
            if (ImGui::Button("Tutup", ImVec2(-1, 32.0f * s))) {
                show_file_modal = false;
            }
        }
    }
    ImGui::End();
}

void EditorUI::render_edit_modal(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.88f, 360.0f * s);
    float h = std::min(io.DisplaySize.y * 0.88f, 310.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Edit Menu##Modal", &show_edit_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImVec2 b_size(-1, 38.0f * s);
        
        bool can_u = manager.can_undo();
        if (!can_u) ImGui::BeginDisabled();
        if (ImGui::Button("Undo (Ctrl+Z)", b_size)) {
            manager.undo();
            show_edit_modal = false;
        }
        if (!can_u) ImGui::EndDisabled();
        
        bool can_r = manager.can_redo();
        if (!can_r) ImGui::BeginDisabled();
        if (ImGui::Button("Redo (Ctrl+Y)", b_size)) {
            manager.redo();
            show_edit_modal = false;
        }
        if (!can_r) ImGui::EndDisabled();
        
        bool has_sel = (manager.get_active_textdraw() != nullptr);
        if (!has_sel) ImGui::BeginDisabled();
        if (ImGui::Button("Duplicate Selected", b_size)) {
            manager.duplicate_selected();
            show_edit_modal = false;
        }
        if (ImGui::Button("Delete Selected", b_size)) {
            manager.delete_selected();
            show_edit_modal = false;
        }
        if (!has_sel) ImGui::EndDisabled();
        
        ImGui::Spacing();
        if (ImGui::Button("Tutup", ImVec2(-1, 32.0f * s))) {
            show_edit_modal = false;
        }
    }
    ImGui::End();
}

void EditorUI::render_view_modal(Viewport& viewport) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.88f, 360.0f * s);
    float h = std::min(io.DisplaySize.y * 0.88f, 290.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("View Settings##Modal", &show_view_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImVec2 b_size(-1, 36.0f * s);
        
        bool is_4_3 = (viewport.aspect_mode == AspectRatioMode::RATIO_4_3);
        if (ImGui::Button(is_4_3 ? "[v] 4:3 Classic SA-MP (Aktif)" : "4:3 Classic SA-MP", b_size)) {
            viewport.aspect_mode = AspectRatioMode::RATIO_4_3;
            viewport.update_viewport_rect();
        }
        if (ImGui::Button(!is_4_3 ? "[v] 16:9 Widescreen (Aktif)" : "16:9 Widescreen", b_size)) {
            viewport.aspect_mode = AspectRatioMode::RATIO_16_9;
            viewport.update_viewport_rect();
        }
        ImGui::Spacing();
        ImGui::Checkbox("Tampilkan Grid Lines", &viewport.enable_grid);
        ImGui::Spacing();
        if (ImGui::Button("Reset Zoom & Pan", b_size)) {
            viewport.reset_view();
            show_view_modal = false;
        }
        if (ImGui::Button("Tutup", ImVec2(-1, 32.0f * s))) {
            show_view_modal = false;
        }
    }
    ImGui::End();
}

void EditorUI::render_bottom_toolbar(TextDrawManager& manager, Viewport& viewport) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::max(1.0f, ui_scale / 1.35f);
    float bar_w = std::min(io.DisplaySize.x - 20.0f, 760.0f * s);
    float bar_h = 56.0f * s;
    
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - bar_w) * 0.5f, io.DisplaySize.y - bar_h - 10.0f));
    ImGui::SetNextWindowSize(ImVec2(bar_w, bar_h));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
                             
    if (ImGui::Begin("##BottomToolbar", nullptr, flags)) {
        ImVec2 b_small(82.0f * s, 40.0f * s);
        ImVec2 b_med(98.0f * s, 40.0f * s);
        ImVec2 b_large(116.0f * s, 40.0f * s);

        if (ImGui::Button("+ TEXT", b_small)) {
            manager.create_text(320.0f, 240.0f, "New Textdraw");
        }
        ImGui::SameLine();
        if (ImGui::Button("+ BOX", b_small)) {
            manager.create_box(320.0f, 240.0f, 140.0f, 40.0f, 0x000000A0);
        }
        ImGui::SameLine();
        if (ImGui::Button("+ SPRITE", b_med)) {
            show_sprite_picker = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ 3D MODEL", b_large)) {
            show_model_picker = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        
        bool has_sel = (manager.get_active_textdraw() != nullptr);
        if (!has_sel) ImGui::BeginDisabled();
        if (ImGui::Button("CLONE", b_small)) {
            manager.duplicate_selected();
        }
        ImGui::SameLine();
        if (ImGui::Button("DEL", ImVec2(65.0f * s, 40.0f * s))) {
            manager.delete_selected();
        }
        if (!has_sel) ImGui::EndDisabled();
    }
    ImGui::End();
}

void EditorUI::render_inspector(TextDrawManager& manager, Viewport& viewport) {
    TextDraw* td = manager.get_active_textdraw();
    if (!td) return; // Only show when a textdraw is selected
    
    ImGuiIO& io = ImGui::GetIO();
    float s = std::max(1.0f, ui_scale / 1.35f);
    float panel_w = std::min(360.0f * s, io.DisplaySize.x * 0.44f);
    float panel_h = io.DisplaySize.y - 75.0f * s;
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panel_w - 12.0f, 32.0f * ui_scale), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel_w, panel_h), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Properties Inspector", nullptr)) {
        // Variable Name & Player Mode
        char var_buf[64];
        strncpy(var_buf, td->variable_name.c_str(), sizeof(var_buf));
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 65.0f * s);
        if (ImGui::InputText("Variable", var_buf, sizeof(var_buf))) {
            td->variable_name = var_buf;
        }
        ImGui::SameLine();
        if (ImGui::Button("EDIT##Var", ImVec2(58.0f * s, 0))) {
            android_show_text_dialog("Edit Variable Name", td->variable_name.c_str(), 2);
        }
        ImGui::Checkbox("Player TextDraw (PlayerText:)", &td->is_player);
        
        ImGui::Separator();
        
        // Coordinates (X, Y)
        ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.15f, 1.0f), "Coordinates (640x480)");
        ImGui::DragFloat("X", &td->x, dpad_step, 0.0f, 640.0f, "%.2f");
        ImGui::DragFloat("Y", &td->y, dpad_step, 0.0f, 480.0f, "%.2f");
        
        ImGui::Separator();
        
        // Font Selector
        const char* font_names[] = {
            "Font 0: Diploma / Gothic",
            "Font 1: Standard (Chalet London)",
            "Font 2: Sub-Header / Futura",
            "Font 3: Pricedown (GTA Title)",
            "Font 4: Sprite (TXD:Texture)",
            "Font 5: 3D Model Preview"
        };
        ImGui::Combo("Font Type", &td->font, font_names, 6);
        
        // Font specific inputs
        if (td->font >= 0 && td->font <= 3) {
            // Text Content with direct EDIT button for native soft keyboard
            char text_buf[256];
            strncpy(text_buf, td->text.c_str(), sizeof(text_buf));
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 65.0f * s);
            if (ImGui::InputText("Text Content", text_buf, sizeof(text_buf))) {
                td->text = text_buf;
            }
            ImGui::SameLine();
            if (ImGui::Button("EDIT##Text", ImVec2(58.0f * s, 0))) {
                android_show_text_dialog("Edit Text Content", td->text.c_str(), 1);
            }
            ImGui::DragFloat("Letter W", &td->letter_width, 0.01f, 0.0f, 5.0f, "%.3f");
            ImGui::DragFloat("Letter H", &td->letter_height, 0.05f, 0.0f, 10.0f, "%.2f");
            
            const char* aligns[] = {"Left", "Center", "Right"};
            int cur_a = (int)td->alignment - 1;
            if (ImGui::Combo("Alignment", &cur_a, aligns, 3)) {
                td->alignment = (TextDrawAlignment)(cur_a + 1);
            }
            ImGui::Checkbox("Proportional", &td->proportional);
        } else if (td->font == 4) { // Sprite
            char sprite_buf[128];
            strncpy(sprite_buf, td->text.c_str(), sizeof(sprite_buf));
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75.0f * s);
            if (ImGui::InputText("Sprite Name", sprite_buf, sizeof(sprite_buf))) {
                td->text = sprite_buf;
            }
            ImGui::SameLine();
            if (ImGui::Button("Browse##Sprite", ImVec2(70.0f * s, 0))) {
                show_sprite_picker = true;
            }
        } else if (td->font == 5) { // 3D Model Preview
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75.0f * s);
            ImGui::InputInt("Model ID", &td->preview_model);
            ImGui::SameLine();
            if (ImGui::Button("Browse##Model", ImVec2(70.0f * s, 0))) {
                show_model_picker = true;
            }
            ImGui::DragFloat("Rot X", &td->rot_x, 1.0f, 0.0f, 360.0f, "%.1f deg");
            ImGui::DragFloat("Rot Y", &td->rot_y, 1.0f, 0.0f, 360.0f, "%.1f deg");
            ImGui::DragFloat("Rot Z", &td->rot_z, 1.0f, 0.0f, 360.0f, "%.1f deg");
            ImGui::DragFloat("Zoom", &td->zoom, 0.05f, 0.1f, 5.0f, "%.2fx");
            
            ImGui::Text("Vehicle Colors:");
            ImGui::PushID("VehCol1");
            if (ImGui::Button("Color 1")) {
                color_target_box = false;
                color_target_veh_col = 1;
                show_carcols_picker = true;
            }
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("ID: %d", td->veh_color1);
            
            ImGui::SameLine();
            ImGui::PushID("VehCol2");
            if (ImGui::Button("Color 2")) {
                color_target_box = false;
                color_target_veh_col = 2;
                show_carcols_picker = true;
            }
            ImGui::PopID();
            ImGui::SameLine();
            ImGui::Text("ID: %d", td->veh_color2);
        }
        
        // TextSize
        ImGui::DragFloat("TextSize W", &td->text_width, 1.0f, 0.0f, 640.0f, "%.1f");
        ImGui::DragFloat("TextSize H", &td->text_height, 1.0f, 0.0f, 480.0f, "%.1f");
        
        ImGui::Separator();
        
        // Colors & Box
        ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.15f, 1.0f), "Appearance & Box");
        
        // Color RGBA
        float col_rgba[4] = {
            ((td->color >> 24) & 0xFF) / 255.0f,
            ((td->color >> 16) & 0xFF) / 255.0f,
            ((td->color >> 8) & 0xFF) / 255.0f,
            (td->color & 0xFF) / 255.0f
        };
        if (ImGui::ColorEdit4("Color", col_rgba)) {
            td->color = ((uint32_t)(col_rgba[0] * 255.0f) << 24) |
                        ((uint32_t)(col_rgba[1] * 255.0f) << 16) |
                        ((uint32_t)(col_rgba[2] * 255.0f) << 8)  |
                        ((uint32_t)(col_rgba[3] * 255.0f));
        }
        
        ImGui::Checkbox("Use Box", &td->use_box);
        if (td->use_box) {
            float box_rgba[4] = {
                ((td->box_color >> 24) & 0xFF) / 255.0f,
                ((td->box_color >> 16) & 0xFF) / 255.0f,
                ((td->box_color >> 8) & 0xFF) / 255.0f,
                (td->box_color & 0xFF) / 255.0f
            };
            if (ImGui::ColorEdit4("Box Color", box_rgba)) {
                td->box_color = ((uint32_t)(box_rgba[0] * 255.0f) << 24) |
                                ((uint32_t)(box_rgba[1] * 255.0f) << 16) |
                                ((uint32_t)(box_rgba[2] * 255.0f) << 8)  |
                                ((uint32_t)(box_rgba[3] * 255.0f));
            }
        }
        
        ImGui::SliderInt("Shadow", &td->shadow, 0, 5);
        ImGui::SliderInt("Outline", &td->outline, 0, 5);
        ImGui::Checkbox("Selectable (Clickable)", &td->selectable);
        
        ImGui::Separator();
        
        // Layering
        ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.15f, 1.0f), "Layer Ordering");
        if (ImGui::Button("Bring to Front")) manager.bring_to_front();
        ImGui::SameLine();
        if (ImGui::Button("Send to Back")) manager.send_to_back();
        if (ImGui::Button("Move Up")) manager.move_layer_up();
        ImGui::SameLine();
        if (ImGui::Button("Move Down")) manager.move_layer_down();
    }
    ImGui::End();
}

void EditorUI::render_dpad_widget(TextDrawManager& manager, Viewport& viewport) {
    TextDraw* td = manager.get_active_textdraw();
    if (!td) return;
    
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.25f);
    float w = 146.0f * s;
    float h = 146.0f * s;
    
    float pos_x = 12.0f * s;
    float pos_y = io.DisplaySize.y - h - 68.0f * s;
    if (pos_y < 42.0f * s) pos_y = 42.0f * s;
    
    ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("Micro D-Pad", nullptr, flags)) {
        float btn_size = 36.0f * s;
        
        // Up
        ImGui::SetCursorPosX((w - btn_size) * 0.5f - 8.0f * s);
        if (ImGui::Button("^##Up", ImVec2(btn_size, btn_size))) {
            manager.move_selected(0.0f, -dpad_step, 0.0f);
        }
        
        // Left, Step, Right
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f * s);
        if (ImGui::Button("<##Left", ImVec2(btn_size, btn_size))) {
            manager.move_selected(-dpad_step, 0.0f, 0.0f);
        }
        ImGui::SameLine();
        char step_lbl[16];
        snprintf(step_lbl, sizeof(step_lbl), "%.1f", dpad_step);
        if (ImGui::Button(step_lbl, ImVec2(btn_size + 4.0f * s, btn_size))) {
            // Cycle steps
            if (dpad_step < 0.3f) dpad_step = 0.5f;
            else if (dpad_step < 0.8f) dpad_step = 1.0f;
            else if (dpad_step < 3.0f) dpad_step = 5.0f;
            else dpad_step = 0.1f;
        }
        ImGui::SameLine();
        if (ImGui::Button(">##Right", ImVec2(btn_size, btn_size))) {
            manager.move_selected(dpad_step, 0.0f, 0.0f);
        }
        
        // Down
        ImGui::SetCursorPosX((w - btn_size) * 0.5f - 8.0f * s);
        if (ImGui::Button("v##Down", ImVec2(btn_size, btn_size))) {
            manager.move_selected(0.0f, dpad_step, 0.0f);
        }
    }
    ImGui::End();
}

void EditorUI::render_sprite_picker(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.92f, 620.0f * s);
    float h = std::min(io.DisplaySize.y * 0.90f, 450.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Browse GTA SA Sprites (591 Sprites)", &show_sprite_picker, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75.0f * s);
        ImGui::InputText("Filter##Sprite", sprite_search_filter, sizeof(sprite_search_filter));
        ImGui::SameLine();
        if (ImGui::Button("KETIK##Sprite", ImVec2(70.0f * s, 0))) {
            android_show_text_dialog("Filter Sprite", sprite_search_filter, 3);
        }
        
        std::string filter = sprite_search_filter;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
        
        ImGui::Separator();
        
        const auto& groups = AssetManager::get().get_sprites();
        float list_h = h - 110.0f * s;
        if (ImGui::BeginChild("SpriteList", ImVec2(0, list_h), true)) {
            for (const auto& [txd, list] : groups) {
                bool group_matches = filter.empty() || (txd.find(filter) != std::string::npos);
                if (!group_matches) {
                    for (const auto& sp : list) {
                        if (sp.name.find(filter) != std::string::npos) {
                            group_matches = true;
                            break;
                        }
                    }
                }
                if (!group_matches) continue;
                
                if (ImGui::CollapsingHeader(txd.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (const auto& sp : list) {
                        if (!filter.empty() && sp.full_name.find(filter) == std::string::npos) continue;
                        
                        if (ImGui::Button(sp.full_name.c_str(), ImVec2(-1, 32.0f * s))) {
                            TextDraw* td = manager.get_active_textdraw();
                            if (td) {
                                td->font = 4;
                                td->text = sp.full_name;
                                td->text_width = (float)sp.width;
                                td->text_height = (float)sp.height;
                            } else {
                                manager.create_sprite(320.0f, 240.0f, (float)sp.width, (float)sp.height, sp.full_name);
                            }
                            show_sprite_picker = false;
                        }
                    }
                }
            }
            ImGui::EndChild();
        }
        
        ImGui::Separator();
        if (ImGui::Button("Tutup", ImVec2(100.0f * s, 32.0f * s))) {
            show_sprite_picker = false;
        }
    }
    ImGui::End();
}

void EditorUI::render_model_picker(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.92f, 620.0f * s);
    float h = std::min(io.DisplaySize.y * 0.90f, 450.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Browse 3D Preview Models", &show_model_picker, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 75.0f * s);
        ImGui::InputText("Filter##Model", model_search_filter, sizeof(model_search_filter));
        ImGui::SameLine();
        if (ImGui::Button("KETIK##Model", ImVec2(70.0f * s, 0))) {
            android_show_text_dialog("Filter 3D Model", model_search_filter, 4);
        }
        
        std::string filter = model_search_filter;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
        
        ImGui::Separator();
        
        float tab_h = h - 110.0f * s;
        if (ImGui::BeginChild("ModelTabsRegion", ImVec2(0, tab_h), true)) {
            if (ImGui::BeginTabBar("ModelTabs")) {
                if (ImGui::BeginTabItem("Vehicles (400 - 611)")) {
                    const auto& vehs = AssetManager::get().get_vehicles();
                    for (const auto& v : vehs) {
                        std::string label = std::to_string(v.id) + ": " + v.name + " (" + v.type + ")";
                        std::string low = label;
                        std::transform(low.begin(), low.end(), low.begin(), ::tolower);
                        if (!filter.empty() && low.find(filter) == std::string::npos) continue;
                        
                        if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_None, ImVec2(-1, 30.0f * s))) {
                            TextDraw* td = manager.get_active_textdraw();
                            if (td) {
                                td->font = 5;
                                td->preview_model = v.id;
                                td->text = std::to_string(v.id);
                                td->rot_x = -16.0f;
                                td->rot_y = 0.0f;
                                td->rot_z = -55.0f;
                                td->zoom = 1.0f;
                                td->veh_color1 = 3;
                                td->veh_color2 = 3;
                            } else {
                                manager.create_preview_model(320.0f, 240.0f, 100.0f, 80.0f, v.id);
                            }
                            show_model_picker = false;
                        }
                    }
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("Skins (0 - 311)")) {
                    const auto& skins = AssetManager::get().get_skins();
                    for (const auto& sk : skins) {
                        std::string label = std::to_string(sk.id) + ": " + sk.name;
                        std::string low = label;
                        std::transform(low.begin(), low.end(), low.begin(), ::tolower);
                        if (!filter.empty() && low.find(filter) == std::string::npos) continue;
                        
                        if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_None, ImVec2(-1, 30.0f * s))) {
                            TextDraw* td = manager.get_active_textdraw();
                            if (td) {
                                td->font = 5;
                                td->preview_model = sk.id;
                                td->text = std::to_string(sk.id);
                                td->rot_x = 0.0f;
                                td->rot_y = 0.0f;
                                td->rot_z = -15.0f;
                                td->zoom = 0.85f;
                            } else {
                                manager.create_preview_model(320.0f, 240.0f, 80.0f, 120.0f, sk.id);
                            }
                            show_model_picker = false;
                        }
                    }
                    ImGui::EndTabItem();
                }
                
                if (ImGui::BeginTabItem("Weapons (321 - 372)")) {
                    struct WeapDef { int id; const char* name; };
                    static const WeapDef kWeaps[] = {
                        {331, "Brass Knuckles"}, {334, "Nightstick"}, {335, "Combat Knife"},
                        {336, "Baseball Bat"}, {337, "Shovel"}, {339, "Katana"},
                        {341, "Chainsaw"}, {342, "Grenade"}, {344, "Molotov Cocktail"},
                        {346, "9mm Pistol (Colt 45)"}, {347, "Silenced 9mm"}, {348, "Desert Eagle"},
                        {349, "Shotgun (Chrome)"}, {350, "Sawnoff Shotgun"}, {351, "Combat Shotgun (SPAS-12)"},
                        {352, "Micro Uzi"}, {353, "MP5"}, {355, "AK-47"}, {356, "M4 Assault Rifle"},
                        {357, "Country Rifle"}, {358, "Sniper Rifle"}, {359, "Rocket Launcher (RPG)"},
                        {360, "Heat-Seeking Rocket"}, {361, "Flamethrower"}, {362, "Minigun"},
                        {371, "Parachute"}, {372, "Tec-9"}
                    };
                    for (const auto& wdef : kWeaps) {
                        std::string label = std::to_string(wdef.id) + ": " + wdef.name;
                        std::string low = label;
                        std::transform(low.begin(), low.end(), low.begin(), ::tolower);
                        if (!filter.empty() && low.find(filter) == std::string::npos) continue;
                        
                        if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_None, ImVec2(-1, 30.0f * s))) {
                            TextDraw* td = manager.get_active_textdraw();
                            if (td) {
                                td->font = 5;
                                td->preview_model = wdef.id;
                                td->text = std::to_string(wdef.id);
                                td->rot_x = -15.0f;
                                td->rot_y = 0.0f;
                                td->rot_z = -45.0f;
                                td->zoom = 1.2f;
                            } else {
                                manager.create_preview_model(320.0f, 240.0f, 90.0f, 75.0f, wdef.id);
                            }
                            show_model_picker = false;
                        }
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::EndChild();
        
        ImGui::Separator();
        if (ImGui::Button("Tutup", ImVec2(100.0f * s, 32.0f * s))) {
            show_model_picker = false;
        }
    }
    ImGui::End();
}

void EditorUI::render_carcols_palette_modal(TextDraw* td, bool target_is_box, int veh_col_index) {
    if (!td) {
        show_carcols_picker = false;
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.90f, 500.0f * s);
    float h = std::min(io.DisplaySize.y * 0.88f, 380.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("SA-MP Color Palette (256 Colors)", &show_carcols_picker, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        const auto& cols = AssetManager::get().get_colors();
        int cols_per_row = 8;
        float btn_w = 48.0f * s;
        float btn_h = 32.0f * s;
        
        float list_h = h - 90.0f * s;
        if (ImGui::BeginChild("ColorGrid", ImVec2(0, list_h), true)) {
            for (size_t i = 0; i < cols.size(); ++i) {
                const auto& c = cols[i];
                ImVec4 btn_col(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 1.0f);
                ImGui::PushID((int)i);
                ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
                
                char id_str[16];
                snprintf(id_str, sizeof(id_str), "%d", c.id);
                if (ImGui::Button(id_str, ImVec2(btn_w, btn_h))) {
                    if (veh_col_index == 1) {
                        td->veh_color1 = c.id;
                    } else if (veh_col_index == 2) {
                        td->veh_color2 = c.id;
                    } else if (target_is_box) {
                        td->box_color = ((uint32_t)c.r << 24) | ((uint32_t)c.g << 16) | ((uint32_t)c.b << 8) | 0x80;
                    } else {
                        td->color = ((uint32_t)c.r << 24) | ((uint32_t)c.g << 16) | ((uint32_t)c.b << 8) | 0xFF;
                    }
                    show_carcols_picker = false;
                }
                ImGui::PopStyleColor();
                ImGui::PopID();
                
                if ((i + 1) % cols_per_row != 0) {
                    ImGui::SameLine();
                }
            }
        }
        ImGui::EndChild();
        
        ImGui::Separator();
        if (ImGui::Button("Tutup", ImVec2(100.0f * s, 32.0f * s))) {
            show_carcols_picker = false;
        }
    }
    ImGui::End();
}

void EditorUI::open_export_modal() {
    show_export_modal = true;
}

void EditorUI::render_export_modal(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.92f, 640.0f * s);
    float h = std::min(io.DisplaySize.y * 0.90f, 460.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Export Pawn Code", &show_export_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        if (export_code_buffer.empty() || ImGui::Button("Perbarui Kode", ImVec2(120.0f * s, 32.0f * s))) {
            export_code_buffer = PawnExporter::export_pawn(manager.get_all_textdraws());
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.65f, 0.15f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
        if (ImGui::Button("📋 Salin ke Clipboard", ImVec2(160.0f * s, 32.0f * s))) {
            ImGui::SetClipboardText(export_code_buffer.c_str());
        }
        ImGui::PopStyleColor(2);
        
        ImGui::Separator();
        
        float out_h = h - 135.0f * s;
        ImGui::InputTextMultiline("##PawnOutput", const_cast<char*>(export_code_buffer.c_str()),
                                  export_code_buffer.size() + 1, ImVec2(-1, out_h),
                                  ImGuiInputTextFlags_ReadOnly);
                                  
        ImGui::Separator();
        if (ImGui::Button("Tutup", ImVec2(100.0f * s, 32.0f * s))) {
            show_export_modal = false;
        }
    }
    ImGui::End();
}

static void parse_and_import_pawn(const char* code, TextDrawManager& manager) {
    if (!code || !*code) return;
    
    std::stringstream ss(code);
    std::string line;
    std::unordered_map<std::string, TextDraw*> created_map;
    
    manager.save_undo_state();
    
    while (std::getline(ss, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);
        
        size_t p_create = line.find("TextDrawCreate(");
        size_t p_player = line.find("CreatePlayerTextDraw(");
        
        if (p_create != std::string::npos) {
            size_t eq = line.find('=');
            std::string var = "";
            if (eq != std::string::npos && eq < p_create) {
                var = line.substr(0, eq);
                var.erase(var.find_last_not_of(" \t") + 1);
                size_t v_start = var.find_last_of(" \t");
                if (v_start != std::string::npos) var = var.substr(v_start + 1);
            }
            
            float x = 320.0f, y = 240.0f;
            char txt[512] = "";
            const char* args = line.c_str() + p_create + 15;
            if (sscanf(args, "%f, %f, \"%[^\"]\"", &x, &y, txt) >= 2) {
                TextDraw* td = manager.create_text(x, y, txt);
                if (td && !var.empty()) {
                    td->variable_name = var;
                    created_map[var] = td;
                }
            }
        } else if (p_player != std::string::npos) {
            size_t eq = line.find('=');
            std::string var = "";
            if (eq != std::string::npos && eq < p_player) {
                var = line.substr(0, eq);
                size_t brk = var.find('[');
                if (brk != std::string::npos) var = var.substr(0, brk);
                var.erase(var.find_last_not_of(" \t") + 1);
                size_t v_start = var.find_last_of(" \t");
                if (v_start != std::string::npos) var = var.substr(v_start + 1);
            }
            
            float x = 320.0f, y = 240.0f;
            char txt[512] = "";
            const char* args = line.c_str() + p_player + 21;
            char pid_dummy[64];
            if (sscanf(args, "%[^,], %f, %f, \"%[^\"]\"", pid_dummy, &x, &y, txt) >= 3) {
                TextDraw* td = manager.create_text(x, y, txt);
                if (td) {
                    td->is_player = true;
                    if (!var.empty()) {
                        td->variable_name = var;
                        created_map[var] = td;
                    }
                }
            }
        }
        
        for (auto& pair : created_map) {
            const std::string& var = pair.first;
            TextDraw* td = pair.second;
            if (!td) continue;
            
            if (line.find(var) != std::string::npos) {
                float f1 = 0, f2 = 0, f3 = 0, f4 = 0;
                int i1 = 0, i2 = 0;
                unsigned int u1 = 0;
                
                if (line.find("LetterSize") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("LetterSize"), "LetterSize(%*[^,], %f, %f)", &f1, &f2) == 2) {
                        td->letter_width = f1;
                        td->letter_height = f2;
                    }
                } else if (line.find("TextSize") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("TextSize"), "TextSize(%*[^,], %f, %f)", &f1, &f2) == 2) {
                        td->text_width = f1;
                        td->text_height = f2;
                    }
                } else if (line.find("Alignment") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("Alignment"), "Alignment(%*[^,], %d)", &i1) == 1) {
                        td->alignment = (TextDrawAlignment)i1;
                    }
                } else if (line.find("Color") != std::string::npos && line.find("BoxColor") == std::string::npos && line.find("VehCol") == std::string::npos) {
                    if (sscanf(line.c_str() + line.find("Color"), "Color(%*[^,], 0x%X)", &u1) == 1 ||
                        sscanf(line.c_str() + line.find("Color"), "Color(%*[^,], %u)", &u1) == 1) {
                        td->color = u1;
                    }
                } else if (line.find("UseBox") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("UseBox"), "UseBox(%*[^,], %d)", &i1) == 1) {
                        td->use_box = (i1 != 0);
                    }
                } else if (line.find("BoxColor") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("BoxColor"), "BoxColor(%*[^,], 0x%X)", &u1) == 1 ||
                        sscanf(line.c_str() + line.find("BoxColor"), "BoxColor(%*[^,], %u)", &u1) == 1) {
                        td->box_color = u1;
                    }
                } else if (line.find("SetShadow") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetShadow"), "SetShadow(%*[^,], %d)", &i1) == 1) {
                        td->shadow = i1;
                    }
                } else if (line.find("SetOutline") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetOutline"), "SetOutline(%*[^,], %d)", &i1) == 1) {
                        td->outline = i1;
                    }
                } else if (line.find("Font") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("Font"), "Font(%*[^,], %d)", &i1) == 1) {
                        td->font = i1;
                    }
                } else if (line.find("SetProportional") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetProportional"), "SetProportional(%*[^,], %d)", &i1) == 1) {
                        td->proportional = (i1 != 0);
                    }
                } else if (line.find("SetSelectable") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetSelectable"), "SetSelectable(%*[^,], %d)", &i1) == 1) {
                        td->selectable = (i1 != 0);
                    }
                } else if (line.find("SetPreviewModel") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetPreviewModel"), "SetPreviewModel(%*[^,], %d)", &i1) == 1) {
                        td->font = 5;
                        td->preview_model = i1;
                        td->text = std::to_string(i1);
                    }
                } else if (line.find("SetPreviewRot") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetPreviewRot"), "SetPreviewRot(%*[^,], %f, %f, %f, %f)", &f1, &f2, &f3, &f4) == 4) {
                        td->rot_x = f1;
                        td->rot_y = f2;
                        td->rot_z = f3;
                        td->zoom = f4;
                    }
                } else if (line.find("SetPreviewVehCol") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetPreviewVehCol"), "SetPreviewVehCol(%*[^,], %d, %d)", &i1, &i2) == 2) {
                        td->veh_color1 = i1;
                        td->veh_color2 = i2;
                    }
                }
            }
        }
    }
}

void EditorUI::open_import_modal() {
    show_import_modal = true;
}

void EditorUI::render_import_modal(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.92f, 620.0f * s);
    float h = std::min(io.DisplaySize.y * 0.90f, 440.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Import Pawn Code", &show_import_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Tempel kode TextDraw Pawn Anda di bawah ini:");
        
        // Prominent 1-tap clipboard paste button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.65f, 0.15f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
        if (ImGui::Button("📋 TEMPEL DARI SALINAN HP (CLIPBOARD)", ImVec2(ImGui::GetContentRegionAvail().x - 85.0f * s, 36.0f * s))) {
            std::string clip = android_get_clipboard();
            if (!clip.empty()) {
                strncpy(import_buffer, clip.c_str(), sizeof(import_buffer) - 1);
            }
        }
        ImGui::PopStyleColor(2);
        
        ImGui::SameLine();
        if (ImGui::Button("✏️ KETIK", ImVec2(80.0f * s, 36.0f * s))) {
            android_show_text_dialog("Input Pawn Code", import_buffer, 5);
        }
        
        ImGui::Spacing();
        
        float input_h = h - 145.0f * s;
        ImGui::InputTextMultiline("##PawnImport", import_buffer, sizeof(import_buffer), ImVec2(-1, input_h));
        
        ImGui::Separator();
        if (ImGui::Button("PROSES IMPORT", ImVec2(140.0f * s, 34.0f * s))) {
            parse_and_import_pawn(import_buffer, manager);
            show_import_modal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Batal", ImVec2(90.0f * s, 34.0f * s))) {
            show_import_modal = false;
        }
    }
    ImGui::End();
}

void EditorUI::open_save_project_modal() {
    show_save_project_modal = true;
}

void EditorUI::open_load_project_modal() {
    show_load_project_modal = true;
}

void EditorUI::render_save_project_modal(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.90f, 480.0f * s);
    float h = std::min(io.DisplaySize.y * 0.85f, 240.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Save Project (.json)", &show_save_project_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Nama File Project:");
        
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 70.0f * s);
        ImGui::InputText(".json##Filename", project_name_buf, sizeof(project_name_buf));
        ImGui::SameLine();
        if (ImGui::Button("KETIK##SaveName", ImVec2(65.0f * s, 0))) {
            android_show_text_dialog("Nama Project", project_name_buf, 6);
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        if (ImGui::Button("Simpan Project", ImVec2(140.0f * s, 36.0f * s))) {
            std::string file_name = std::string(project_name_buf) + ".json";
            std::string full_path = storage_path.empty() ? file_name : (storage_path + "/" + file_name);
            manager.save_project_to_file(full_path, project_name_buf);
            show_save_project_modal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Batal", ImVec2(90.0f * s, 36.0f * s))) {
            show_save_project_modal = false;
        }
    }
    ImGui::End();
}

void EditorUI::render_load_project_modal(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.90f, 520.0f * s);
    float h = std::min(io.DisplaySize.y * 0.88f, 380.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Open Project (.json)", &show_load_project_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Daftar Project Tersimpan:");
        ImGui::Separator();
        
        float list_h = h - 110.0f * s;
        if (ImGui::BeginChild("ProjectList", ImVec2(0, list_h), true)) {
            DIR* dir = opendir(storage_path.empty() ? "." : storage_path.c_str());
            if (dir) {
                struct dirent* ent;
                bool found_any = false;
                while ((ent = readdir(dir)) != nullptr) {
                    std::string fname = ent->d_name;
                    if (fname.size() > 5 && fname.substr(fname.size() - 5) == ".json") {
                        found_any = true;
                        std::string label = "📂 " + fname;
                        if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_None, ImVec2(-1, 36.0f * s))) {
                            std::string full_path = storage_path.empty() ? fname : (storage_path + "/" + fname);
                            manager.load_project_from_file(full_path);
                            show_load_project_modal = false;
                            break;
                        }
                    }
                }
                closedir(dir);
                if (!found_any) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Belum ada project .json yang tersimpan.");
                }
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "Tidak dapat membuka folder project.");
            }
        }
        ImGui::EndChild();
        
        ImGui::Separator();
        if (ImGui::Button("Tutup", ImVec2(100.0f * s, 34.0f * s))) {
            show_load_project_modal = false;
        }
    }
    ImGui::End();
}

void EditorUI::set_dialog_text(int field_id, const std::string& text, TextDrawManager& manager) {
    TextDraw* td = manager.get_active_textdraw();
    if (field_id == 1 && td) {
        td->text = text;
    } else if (field_id == 2 && td) {
        td->variable_name = text;
    } else if (field_id == 3) {
        strncpy(sprite_search_filter, text.c_str(), sizeof(sprite_search_filter) - 1);
    } else if (field_id == 4) {
        strncpy(model_search_filter, text.c_str(), sizeof(model_search_filter) - 1);
    } else if (field_id == 5) {
        strncpy(import_buffer, text.c_str(), sizeof(import_buffer) - 1);
    } else if (field_id == 6) {
        strncpy(project_name_buf, text.c_str(), sizeof(project_name_buf) - 1);
    }
}

