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
    
    // Modals & Panels
    if (show_file_modal) render_file_modal(manager);
    if (show_edit_modal) render_edit_modal(manager);
    if (show_view_modal) render_view_modal(viewport);
    if (show_layers_panel) render_layers_panel(manager);
    if (show_group_modal) render_group_modal(manager);
    if (show_trash_modal) render_trash_modal(manager);
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

    // Center Crosshair Guides
    if (viewport.show_center_guides) {
        float cx_s, cy_s;
        viewport.samp_to_screen(320.0f, 240.0f, cx_s, cy_s);
        draw_list->AddLine(ImVec2(cx_s, c_y1), ImVec2(cx_s, c_y2), IM_COL32(0, 230, 255, 45), 1.0f);
        draw_list->AddLine(ImVec2(c_x1, cy_s), ImVec2(c_x2, cy_s), IM_COL32(0, 230, 255, 45), 1.0f);
    }

    // Safe Zone Guides (5% margin)
    if (viewport.show_safe_zone) {
        float sz_x1, sz_y1, sz_x2, sz_y2;
        viewport.samp_to_screen(32.0f, 24.0f, sz_x1, sz_y1);
        viewport.samp_to_screen(608.0f, 456.0f, sz_x2, sz_y2);
        draw_list->AddRect(ImVec2(sz_x1, sz_y1), ImVec2(sz_x2, sz_y2), IM_COL32(255, 100, 100, 70), 0.0f, 0, 1.0f);
    }

    // GTA SA HUD Template Reference Overlay
    if (viewport.show_hud_overlay) {
        // Radar circle at bottom left (GTA SA default: x=55..140, y=345..430)
        float r_sx, r_sy;
        viewport.samp_to_screen(55.0f, 345.0f, r_sx, r_sy);
        float r_sw = viewport.samp_to_screen_scale_x(85.0f);
        float r_sh = viewport.samp_to_screen_scale_y(85.0f);
        draw_list->AddCircle(ImVec2(r_sx + r_sw * 0.5f, r_sy + r_sh * 0.5f), r_sw * 0.5f, IM_COL32(100, 200, 255, 90), 32, 1.5f);
        draw_list->AddText(ImVec2(r_sx + r_sw * 0.5f - 4, r_sy + 4), IM_COL32(255, 100, 100, 140), "N");
        draw_list->AddText(ImVec2(r_sx + 10, r_sy + r_sh * 0.5f - 6), IM_COL32(150, 180, 200, 120), "RADAR HUD");

        // Top-Right Weapon Box
        float w_sx, w_sy;
        viewport.samp_to_screen(545.0f, 22.0f, w_sx, w_sy);
        float w_sw = viewport.samp_to_screen_scale_x(65.0f);
        float w_sh = viewport.samp_to_screen_scale_y(55.0f);
        draw_list->AddRect(ImVec2(w_sx, w_sy), ImVec2(w_sx + w_sw, w_sy + w_sh), IM_COL32(255, 200, 100, 90), 2.0f, 0, 1.5f);
        draw_list->AddText(ImVec2(w_sx + 8, w_sy + 18), IM_COL32(255, 200, 100, 120), "WEAPON");

        // Health & Armor bars
        float h_sx, h_sy;
        viewport.samp_to_screen(545.0f, 82.0f, h_sx, h_sy);
        float h_sw = viewport.samp_to_screen_scale_x(65.0f);
        float h_sh = viewport.samp_to_screen_scale_y(9.0f);
        draw_list->AddRectFilled(ImVec2(h_sx, h_sy), ImVec2(h_sx + h_sw, h_sy + h_sh), IM_COL32(220, 50, 50, 70));
        draw_list->AddRect(ImVec2(h_sx, h_sy), ImVec2(h_sx + h_sw, h_sy + h_sh), IM_COL32(220, 50, 50, 140));

        float a_sx, a_sy;
        viewport.samp_to_screen(545.0f, 94.0f, a_sx, a_sy);
        draw_list->AddRectFilled(ImVec2(a_sx, a_sy), ImVec2(a_sx + h_sw, a_sy + h_sh), IM_COL32(220, 220, 220, 70));
        draw_list->AddRect(ImVec2(a_sx, a_sy), ImVec2(a_sx + h_sw, a_sy + h_sh), IM_COL32(220, 220, 220, 140));

        // Money Counter
        float m_sx, m_sy;
        viewport.samp_to_screen(530.0f, 108.0f, m_sx, m_sy);
        draw_list->AddText(ImVec2(m_sx, m_sy), IM_COL32(60, 180, 80, 130), "$00099999");
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
        
        // A. Draw Box if enabled (using SA-MP TextDrawTextSize semantics)
        if (td.use_box) {
            uint32_t c = td.box_color;
            ImU32 im_col = IM_COL32((c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
            float bx1, by1, bx2, by2;
            td.get_bounds(bx1, by1, bx2, by2);
            float sbx1, sby1, sbx2, sby2;
            viewport.samp_to_screen(bx1, by1, sbx1, sby1);
            viewport.samp_to_screen(bx2, by2, sbx2, sby2);
            draw_list->AddRectFilled(ImVec2(sbx1, sby1), ImVec2(sbx2, sby2), im_col);
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
                draw_list->AddRect(ImVec2(sx, sy), ImVec2(sx + sw, sy + sh), IM_COL32(255, 100, 100, 200));
                draw_list->AddText(ImVec2(sx + 4, sy + 4), IM_COL32(255, 200, 200, 255), td.text.c_str());
            }
        } else if (td.font == 5) { // 3D Model Preview
            GLuint tex = DffRenderer::get().render_to_texture(
                td.preview_model, td.rot_x, td.rot_y, td.rot_z, td.zoom,
                td.veh_color1, td.veh_color2
            );
            if (tex != 0) {
                draw_list->AddImage((ImTextureID)(intptr_t)tex,
                                   ImVec2(sx, sy), ImVec2(sx + sw, sy + sh),
                                   ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            } else {
                draw_list->AddRect(ImVec2(sx, sy), ImVec2(sx + sw, sy + sh), IM_COL32(255, 180, 50, 200));
                draw_list->AddText(ImVec2(sx + 4, sy + 4), IM_COL32(255, 255, 255, 255), "3D Model");
            }
        } else { // Fonts 0, 1, 2, 3 (Text)
            // SA-MP calibration:
            // 1.0 letter_height unit ≈ 9.3px at native 640×480 canvas
            // This was empirically derived from NexTDE reference (letter_height=1.5 → ~14px tall in-game)
            float px_per_samp = viewport.canvas_screen_h / 480.0f;
            float font_size = std::max(4.0f, td.letter_height * 9.3f * px_per_samp);
            
            // SA-MP line spacing: each ~n~ line is spaced by letter_height units in SA-MP coords
            // In SA-MP: vertical gap between lines ≈ letter_height * 15px at native res
            float line_height = td.letter_height * 15.0f * px_per_samp;
            
            uint32_t bg_c = td.background_color;
            ImU32 im_bg = IM_COL32((bg_c >> 24) & 0xFF, (bg_c >> 16) & 0xFF, (bg_c >> 8) & 0xFF, bg_c & 0xFF);
            
            // Parse SA-MP formatting tags (~r~, ~g~, ~b~, ~w~, ~y~, ~p~, ~l~, ~s~, ~h~, ~n~)
            auto spans = SampColorParser::parse(td.text, td.color);
            
            // Group spans by lines
            std::vector<std::vector<TextSpan>> lines;
            std::vector<TextSpan> current_line;
            for (const auto& span : spans) {
                if (span.is_newline) {
                    lines.push_back(current_line);
                    current_line.clear();
                } else if (!span.text.empty()) {
                    current_line.push_back(span);
                }
            }
            lines.push_back(current_line);
            
            float cur_y = sy;
            
            for (const auto& line : lines) {
                // Calculate total line width for accurate SA-MP alignment
                float line_width = 0.0f;
                for (const auto& span : line) {
                    ImVec2 sz = ImGui::GetFont()->CalcTextSizeA(font_size, FLT_MAX, 0.0f, span.text.c_str());
                    line_width += sz.x;
                }
                
                float cur_x = sx;
                if (td.alignment == TextDrawAlignment::CENTER) {
                    // In SA-MP, 'x' is the CENTER point for centered text
                    cur_x = sx - line_width * 0.5f;
                } else if (td.alignment == TextDrawAlignment::RIGHT) {
                    // In SA-MP, 'x' is the RIGHT edge for right-aligned text
                    cur_x = sx - line_width;
                }
                
                for (const auto& span : line) {
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
                    
                    ImVec2 span_size = ImGui::GetFont()->CalcTextSizeA(font_size, FLT_MAX, 0.0f, span.text.c_str());
                    cur_x += span_size.x;
                }
                
                cur_y += line_height;
            }
        }
        
        // C. Selection Bounding Box & Handles
        if (td.is_selected || td.is_grouped) {
            float x1, y1, x2, y2;
            td.get_bounds(x1, y1, x2, y2);
            float b_sx1, b_sy1, b_sx2, b_sy2;
            viewport.samp_to_screen(x1, y1, b_sx1, b_sy1);
            viewport.samp_to_screen(x2, y2, b_sx2, b_sy2);
            
            ImU32 border_col = td.is_selected ? IM_COL32(0, 230, 255, 255) : IM_COL32(255, 180, 20, 180);
            draw_list->AddRect(ImVec2(b_sx1, b_sy1), ImVec2(b_sx2, b_sy2), border_col, 2.0f, 0, td.is_selected ? 1.5f : 1.0f);
            
            if (td.is_selected) {
                // Corner handles
                const float h_size = 5.0f;
                draw_list->AddRectFilled(ImVec2(b_sx1 - h_size, b_sy1 - h_size), ImVec2(b_sx1 + h_size, b_sy1 + h_size), border_col);
                draw_list->AddRectFilled(ImVec2(b_sx2 - h_size, b_sy1 - h_size), ImVec2(b_sx2 + h_size, b_sy1 + h_size), border_col);
                draw_list->AddRectFilled(ImVec2(b_sx1 - h_size, b_sy2 - h_size), ImVec2(b_sx1 + h_size, b_sy2 + h_size), border_col);
                draw_list->AddRectFilled(ImVec2(b_sx2 - h_size, b_sy2 - h_size), ImVec2(b_sx2 + h_size, b_sy2 + h_size), border_col);
                
                // Coordinate & name tag
                char tag[96];
                snprintf(tag, sizeof(tag), "%s (%.1f, %.1f)", td.variable_name.c_str(), td.x, td.y);
                draw_list->AddText(ImVec2(b_sx1, b_sy1 - 16.0f), border_col, tag);
            }
        }
    }
}

void EditorUI::render_top_bar(TextDrawManager& manager, Viewport& viewport) {
    if (ImGui::BeginMainMenuBar()) {
        ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.15f, 1.0f), "GTA SA-MP");
        ImGui::Separator();
        
        // Touch-safe top buttons
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
        
        // Layers Panel Toggle Button
        size_t total_tds = manager.get_all_textdraws().size();
        std::string layers_label = "LAYERS (" + std::to_string(total_tds) + ")";
        if (show_layers_panel) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.55f, 0.85f, 0.90f));
        }
        if (ImGui::Button(layers_label.c_str())) {
            show_layers_panel = !show_layers_panel;
        }
        if (show_layers_panel) {
            ImGui::PopStyleColor();
        }
        
        // Group Action Button
        int sel_count = manager.get_selected_count();
        std::string grp_label = "GROUP" + (sel_count > 1 ? (" (" + std::to_string(sel_count) + ")") : "");
        if (sel_count > 1) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.90f, 0.55f, 0.12f, 0.90f));
        }
        if (ImGui::Button(grp_label.c_str())) {
            show_group_modal = true;
        }
        if (sel_count > 1) {
            ImGui::PopStyleColor();
        }
        
        // Trash (Recycle Bin) Button
        size_t trash_count = manager.get_trash_count();
        if (trash_count > 0) {
            std::string trash_label = "TRASH (" + std::to_string(trash_count) + ")";
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.20f, 0.20f, 0.90f));
            if (ImGui::Button(trash_label.c_str())) {
                show_trash_modal = true;
            }
            ImGui::PopStyleColor();
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
    float w = std::min(io.DisplaySize.x * 0.90f, 380.0f * s);
    float h = std::min(io.DisplaySize.y * 0.90f, 380.0f * s);
    
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
        ImGui::Checkbox("GTA SA HUD Template Overlay", &viewport.show_hud_overlay);
        ImGui::Checkbox("Center Crosshair Guides (320x240)", &viewport.show_center_guides);
        ImGui::Checkbox("Screen Safe Zone Frame", &viewport.show_safe_zone);
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
    float bar_w = std::min(io.DisplaySize.x - 20.0f, 820.0f * s);
    float bar_h = 56.0f * s;
    
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - bar_w) * 0.5f, io.DisplaySize.y - bar_h - 10.0f));
    ImGui::SetNextWindowSize(ImVec2(bar_w, bar_h));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
                             
    if (ImGui::Begin("##BottomToolbar", nullptr, flags)) {
        ImVec2 b_small(78.0f * s, 40.0f * s);
        ImVec2 b_med(94.0f * s, 40.0f * s);
        ImVec2 b_large(112.0f * s, 40.0f * s);

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
        
        if (ImGui::Button("LAYERS", b_med)) {
            show_layers_panel = !show_layers_panel;
        }
        ImGui::SameLine();
        
        int sel_count = manager.get_selected_count();
        if (ImGui::Button("GROUP", b_small)) {
            show_group_modal = true;
        }
        ImGui::SameLine();
        
        bool has_sel = (manager.get_active_textdraw() != nullptr);
        if (!has_sel) ImGui::BeginDisabled();
        if (ImGui::Button("CLONE", b_small)) {
            manager.duplicate_selected();
        }
        ImGui::SameLine();
        if (ImGui::Button("DEL", ImVec2(60.0f * s, 40.0f * s))) {
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
        
        // Grouping ID
        ImGui::SetNextItemWidth(80.0f * s);
        if (ImGui::InputInt("Group ID", &td->group_id)) {
            if (td->group_id < 0) td->group_id = 0;
            td->is_grouped = (td->group_id > 0);
        }
        ImGui::SameLine();
        if (td->is_grouped) {
            ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.15f, 1.0f), "[Grouped]");
        } else {
            ImGui::TextDisabled("[No Group]");
        }
        
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
            
            // SA-MP Color Tag Quick Insert Chips
            ImGui::Text("Color Tags:");
            const auto& samp_tags = SampColorParser::get_available_tags();
            for (size_t ti = 0; ti < samp_tags.size(); ++ti) {
                const auto& tag_def = samp_tags[ti];
                ImGui::PushID((int)ti);
                uint32_t tc = tag_def.color_preview;
                ImVec4 btn_col(((tc >> 24) & 0xFF) / 255.0f, ((tc >> 16) & 0xFF) / 255.0f, ((tc >> 8) & 0xFF) / 255.0f, 0.85f);
                ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.08f, 0.08f, 0.10f, 1.0f));
                if (ImGui::SmallButton(tag_def.tag.c_str())) {
                    td->text += tag_def.tag;
                }
                ImGui::PopStyleColor(2);
                if ((ti + 1) % 7 != 0 && ti + 1 < samp_tags.size()) {
                    ImGui::SameLine();
                }
                ImGui::PopID();
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
        
        // TextSize - meaning depends on alignment (SA-MP rule)
        if (td->font == 4 || td->font == 5) {
            ImGui::DragFloat("TextSize W (width)", &td->text_width, 1.0f, 0.0f, 640.0f, "%.1f");
            ImGui::DragFloat("TextSize H (height)", &td->text_height, 1.0f, 0.0f, 480.0f, "%.1f");
        } else if (td->alignment == TextDrawAlignment::LEFT) {
            ImGui::DragFloat("TextSize X (right edge)", &td->text_width, 1.0f, 0.0f, 640.0f, "%.1f");
            ImGui::DragFloat("TextSize Y (bottom edge)", &td->text_height, 1.0f, 0.0f, 480.0f, "%.1f");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("LEFT: TextDrawTextSize(td, X_right, Y_bottom) -- absolute coords");
        } else if (td->alignment == TextDrawAlignment::CENTER) {
            ImGui::DragFloat("TextSize X (box width)", &td->text_width, 1.0f, 0.0f, 640.0f, "%.1f");
            ImGui::DragFloat("TextSize Y (box height)", &td->text_height, 1.0f, 0.0f, 480.0f, "%.1f");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("CENTER: TextDrawTextSize(td, width, height) -- box size");
        } else {
            ImGui::DragFloat("TextSize X (left edge)", &td->text_width, 1.0f, 0.0f, 640.0f, "%.1f");
            ImGui::DragFloat("TextSize Y (top edge)", &td->text_height, 1.0f, 0.0f, 480.0f, "%.1f");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("RIGHT: TextDrawTextSize(td, X_left, Y_top) -- x is right edge");
        }
        if (td->font >= 0 && td->font <= 3) {
            if (ImGui::Button("Auto-fit TextSize##Btn", ImVec2(-1, 30.0f * s))) {
                td->auto_calculate_text_size();
            }
        }

        
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

        // Shadow / Outline Background Color
        float bg_rgba[4] = {
            ((td->background_color >> 24) & 0xFF) / 255.0f,
            ((td->background_color >> 16) & 0xFF) / 255.0f,
            ((td->background_color >> 8) & 0xFF) / 255.0f,
            (td->background_color & 0xFF) / 255.0f
        };
        if (ImGui::ColorEdit4("Shadow/Outline Col", bg_rgba)) {
            td->background_color = ((uint32_t)(bg_rgba[0] * 255.0f) << 24) |
                                   ((uint32_t)(bg_rgba[1] * 255.0f) << 16) |
                                   ((uint32_t)(bg_rgba[2] * 255.0f) << 8)  |
                                   ((uint32_t)(bg_rgba[3] * 255.0f));
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
    float w = std::min(io.DisplaySize.x * 0.94f, 660.0f * s);
    float h = std::min(io.DisplaySize.y * 0.92f, 480.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Export Pawn Code", &show_export_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        if (export_code_buffer.empty() || ImGui::Button("Perbarui Kode", ImVec2(120.0f * s, 32.0f * s))) {
            export_code_buffer = PawnExporter::export_pawn(manager.get_all_textdraws(), export_wrap_functions, export_only_selected);
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.65f, 0.15f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.10f, 0.10f, 0.12f, 1.00f));
        if (ImGui::Button("Salin ke Clipboard", ImVec2(150.0f * s, 32.0f * s))) {
            ImGui::SetClipboardText(export_code_buffer.c_str());
        }
        ImGui::PopStyleColor(2);
        
        bool changed = false;
        if (ImGui::Checkbox("Hanya Yang Dipilih (Group)", &export_only_selected)) changed = true;
        ImGui::SameLine();
        if (ImGui::Checkbox("Bungkus OnGameModeInit / OnPlayerConnect", &export_wrap_functions)) changed = true;
        if (changed) {
            export_code_buffer = PawnExporter::export_pawn(manager.get_all_textdraws(), export_wrap_functions, export_only_selected);
        }
        
        ImGui::Separator();
        
        float out_h = h - 165.0f * s;
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

static uint32_t parse_samp_color_value(const std::string& str) {
    std::string s = str;
    s.erase(0, s.find_first_not_of(" \t\r\n"));
    size_t last = s.find_last_not_of(" \t\r\n;)");
    if (last != std::string::npos) s = s.substr(0, last + 1);
    if (s.empty()) return 0xFFFFFFFF;
    
    if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
        try {
            return (uint32_t)std::stoul(s, nullptr, 16);
        } catch (...) { return 0xFFFFFFFF; }
    }
    try {
        long long val = std::stoll(s);
        return (uint32_t)val;
    } catch (...) {
        return 0xFFFFFFFF;
    }
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
        if (line.rfind("//", 0) == 0 || line.rfind("/*", 0) == 0) continue;
        
        size_t p_create = line.find("TextDrawCreate(");
        size_t p_player = line.find("CreatePlayerTextDraw(");
        
        if (p_create != std::string::npos) {
            size_t eq = line.find('=');
            std::string var = "";
            if (eq != std::string::npos && eq < p_create) {
                var = line.substr(0, eq);
                var.erase(var.find_last_not_of(" \t") + 1);
                size_t v_start = var.find_last_of(" \t*:");
                if (v_start != std::string::npos) var = var.substr(v_start + 1);
            }
            
            float x = 320.0f, y = 240.0f;
            std::string txt = "New Textdraw";
            size_t q1 = line.find('"', p_create);
            size_t q2 = line.rfind('"');
            if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                txt = line.substr(q1 + 1, q2 - q1 - 1);
                std::string coords = line.substr(p_create + 15, q1 - (p_create + 15));
                sscanf(coords.c_str(), "%f, %f", &x, &y);
            } else {
                sscanf(line.c_str() + p_create + 15, "%f, %f", &x, &y);
            }
            
            TextDraw* td = manager.create_text(x, y, txt);
            if (td && !var.empty()) {
                td->variable_name = var;
                created_map[var] = td;
            }
        } else if (p_player != std::string::npos) {
            size_t eq = line.find('=');
            std::string var = "";
            if (eq != std::string::npos && eq < p_player) {
                var = line.substr(0, eq);
                size_t brk = var.find('[');
                if (brk != std::string::npos) var = var.substr(0, brk);
                var.erase(var.find_last_not_of(" \t") + 1);
                size_t v_start = var.find_last_of(" \t*:");
                if (v_start != std::string::npos) var = var.substr(v_start + 1);
            }
            
            float x = 320.0f, y = 240.0f;
            std::string txt = "New Textdraw";
            size_t q1 = line.find('"', p_player);
            size_t q2 = line.rfind('"');
            if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                txt = line.substr(q1 + 1, q2 - q1 - 1);
                std::string args_before = line.substr(p_player + 21, q1 - (p_player + 21));
                sscanf(args_before.c_str(), "%*[^,], %f, %f", &x, &y);
            } else {
                sscanf(line.c_str() + p_player + 21, "%*[^,], %f, %f", &x, &y);
            }
            
            TextDraw* td = manager.create_text(x, y, txt);
            if (td) {
                td->is_player = true;
                if (!var.empty()) {
                    td->variable_name = var;
                    created_map[var] = td;
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
                
                if (line.find("LetterSize") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("LetterSize"), "LetterSize(%*[^,], %f, %f)", &f1, &f2) == 2 ||
                        sscanf(line.c_str() + line.find("LetterSize"), "LetterSize(%*[^,],%*[^,], %f, %f)", &f1, &f2) == 2) {
                        td->letter_width = f1;
                        td->letter_height = f2;
                    }
                } else if (line.find("TextSize") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("TextSize"), "TextSize(%*[^,], %f, %f)", &f1, &f2) == 2 ||
                        sscanf(line.c_str() + line.find("TextSize"), "TextSize(%*[^,],%*[^,], %f, %f)", &f1, &f2) == 2) {
                        td->text_width = f1;
                        td->text_height = f2;
                    }
                } else if (line.find("Alignment") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("Alignment"), "Alignment(%*[^,], %d)", &i1) == 1 ||
                        sscanf(line.c_str() + line.find("Alignment"), "Alignment(%*[^,],%*[^,], %d)", &i1) == 1) {
                        td->alignment = (TextDrawAlignment)i1;
                    }
                } else if (line.find("BackgroundColor") != std::string::npos) {
                    size_t comma = line.find_last_of(',');
                    size_t close = line.rfind(')');
                    if (comma != std::string::npos && close != std::string::npos && close > comma) {
                        td->background_color = parse_samp_color_value(line.substr(comma + 1, close - comma - 1));
                    }
                } else if (line.find("BoxColor") != std::string::npos) {
                    size_t comma = line.find_last_of(',');
                    size_t close = line.rfind(')');
                    if (comma != std::string::npos && close != std::string::npos && close > comma) {
                        td->box_color = parse_samp_color_value(line.substr(comma + 1, close - comma - 1));
                    }
                } else if (line.find("Color") != std::string::npos && line.find("VehCol") == std::string::npos) {
                    size_t comma = line.find_last_of(',');
                    size_t close = line.rfind(')');
                    if (comma != std::string::npos && close != std::string::npos && close > comma) {
                        td->color = parse_samp_color_value(line.substr(comma + 1, close - comma - 1));
                    }
                } else if (line.find("UseBox") != std::string::npos) {
                    td->use_box = (line.find("1") != std::string::npos || line.find("true") != std::string::npos);
                } else if (line.find("SetShadow") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetShadow"), "SetShadow(%*[^,], %d)", &i1) == 1 ||
                        sscanf(line.c_str() + line.find("SetShadow"), "SetShadow(%*[^,],%*[^,], %d)", &i1) == 1) {
                        td->shadow = i1;
                    }
                } else if (line.find("SetOutline") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetOutline"), "SetOutline(%*[^,], %d)", &i1) == 1 ||
                        sscanf(line.c_str() + line.find("SetOutline"), "SetOutline(%*[^,],%*[^,], %d)", &i1) == 1) {
                        td->outline = i1;
                    }
                } else if (line.find("Font") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("Font"), "Font(%*[^,], %d)", &i1) == 1 ||
                        sscanf(line.c_str() + line.find("Font"), "Font(%*[^,],%*[^,], %d)", &i1) == 1) {
                        td->font = i1;
                    }
                } else if (line.find("SetProportional") != std::string::npos) {
                    td->proportional = (line.find("1") != std::string::npos || line.find("true") != std::string::npos);
                } else if (line.find("SetSelectable") != std::string::npos) {
                    td->selectable = (line.find("1") != std::string::npos || line.find("true") != std::string::npos);
                } else if (line.find("SetPreviewModel") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetPreviewModel"), "SetPreviewModel(%*[^,], %d)", &i1) == 1 ||
                        sscanf(line.c_str() + line.find("SetPreviewModel"), "SetPreviewModel(%*[^,],%*[^,], %d)", &i1) == 1) {
                        td->font = 5;
                        td->preview_model = i1;
                        td->text = std::to_string(i1);
                    }
                } else if (line.find("SetPreviewRot") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetPreviewRot"), "SetPreviewRot(%*[^,], %f, %f, %f, %f)", &f1, &f2, &f3, &f4) == 4 ||
                        sscanf(line.c_str() + line.find("SetPreviewRot"), "SetPreviewRot(%*[^,],%*[^,], %f, %f, %f, %f)", &f1, &f2, &f3, &f4) == 4) {
                        td->rot_x = f1;
                        td->rot_y = f2;
                        td->rot_z = f3;
                        td->zoom = f4;
                    }
                } else if (line.find("SetPreviewVehCol") != std::string::npos) {
                    if (sscanf(line.c_str() + line.find("SetPreviewVehCol"), "SetPreviewVehCol(%*[^,], %d, %d)", &i1, &i2) == 2 ||
                        sscanf(line.c_str() + line.find("SetPreviewVehCol"), "SetPreviewVehCol(%*[^,],%*[^,], %d, %d)", &i1, &i2) == 2) {
                        td->veh_color1 = i1;
                        td->veh_color2 = i2;
                    }
                } else if (line.find("SetString") != std::string::npos) {
                    size_t q1 = line.find('"');
                    size_t q2 = line.rfind('"');
                    if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1) {
                        td->text = line.substr(q1 + 1, q2 - q1 - 1);
                    }
                }
            }
        }
    }
}

void EditorUI::render_layers_panel(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.90f, 440.0f * s);
    float h = std::min(io.DisplaySize.y * 0.88f, 520.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(12.0f * s, 42.0f * s), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Layers & TextDraw List", &show_layers_panel)) {
        if (ImGui::Button("Semua##All", ImVec2(70.0f * s, 28.0f * s))) manager.select_all();
        ImGui::SameLine();
        if (ImGui::Button("Batal##Clear", ImVec2(70.0f * s, 28.0f * s))) manager.clear_selection();
        ImGui::SameLine();
        if (ImGui::Button("Invert##Inv", ImVec2(60.0f * s, 28.0f * s))) manager.invert_selection();
        ImGui::SameLine();
        if (ImGui::Button("Group Menu##Grp", ImVec2(105.0f * s, 28.0f * s))) show_group_modal = true;
        
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##LayerSearch", "Cari Variable atau Teks...", layer_search_filter, sizeof(layer_search_filter));
        
        std::string filter = layer_search_filter;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
        
        ImGui::Separator();
        
        if (ImGui::BeginChild("##LayersListChild", ImVec2(0, -38.0f * s), true)) {
            auto& list = manager.get_all_textdraws();
            for (int i = (int)list.size() - 1; i >= 0; --i) {
                auto& td = list[i];
                
                if (!filter.empty()) {
                    std::string var_low = td.variable_name;
                    std::transform(var_low.begin(), var_low.end(), var_low.begin(), ::tolower);
                    std::string txt_low = td.text;
                    std::transform(txt_low.begin(), txt_low.end(), txt_low.begin(), ::tolower);
                    if (var_low.find(filter) == std::string::npos && txt_low.find(filter) == std::string::npos) {
                        continue;
                    }
                }
                
                ImGui::PushID(td.id);
                
                bool sel = td.is_selected;
                if (ImGui::Checkbox("##Sel", &sel)) {
                    manager.toggle_selection(td.id);
                }
                ImGui::SameLine();
                
                if (ImGui::SmallButton(td.is_visible ? "V" : "-")) {
                    td.is_visible = !td.is_visible;
                }
                ImGui::SameLine();
                
                if (ImGui::SmallButton(td.is_locked ? "L" : "U")) {
                    td.is_locked = !td.is_locked;
                }
                ImGui::SameLine();
                
                const char* type_str = "TXT";
                ImVec4 badge_col = ImVec4(0.35f, 0.75f, 1.0f, 1.0f);
                if (td.use_box) { type_str = "BOX"; badge_col = ImVec4(0.85f, 0.45f, 1.0f, 1.0f); }
                else if (td.font == 4) { type_str = "SPR"; badge_col = ImVec4(0.45f, 0.90f, 0.55f, 1.0f); }
                else if (td.font == 5) { type_str = "3D"; badge_col = ImVec4(1.0f, 0.75f, 0.25f, 1.0f); }
                
                ImGui::TextColored(badge_col, "[%s]", type_str);
                ImGui::SameLine();
                
                char label[128];
                std::string prev_txt = td.text.size() > 14 ? (td.text.substr(0, 14) + "..") : td.text;
                snprintf(label, sizeof(label), "%s (%s)", td.variable_name.c_str(), prev_txt.c_str());
                
                float avail = ImGui::GetContentRegionAvail().x - 65.0f * s;
                if (ImGui::Selectable(label, td.is_selected, 0, ImVec2(avail > 80.0f ? avail : 80.0f, 0))) {
                    manager.select_single(td.id);
                }
                ImGui::SameLine();
                
                if (ImGui::SmallButton("^") && i < (int)list.size() - 1) {
                    manager.move_layer(i, i + 1);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("v") && i > 0) {
                    manager.move_layer(i, i - 1);
                }
                
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        
        ImGui::Text("Total: %zu | Dipilih: %d", manager.get_all_textdraws().size(), manager.get_selected_count());
        ImGui::SameLine(ImGui::GetWindowWidth() - 95.0f * s);
        if (ImGui::Button("Tutup##Layers", ImVec2(85.0f * s, 26.0f * s))) {
            show_layers_panel = false;
        }
    }
    ImGui::End();
}

void EditorUI::render_group_modal(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.92f, 490.0f * s);
    float h = std::min(io.DisplaySize.y * 0.90f, 490.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Group Operations & Tools (Gruplama)", &show_group_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        int sel_count = manager.get_selected_count();
        ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.15f, 1.0f), "TextDraw Terpilih: %d / %zu", sel_count, manager.get_all_textdraws().size());
        ImGui::Separator();
        
        if (sel_count == 0) {
            ImGui::TextWrapped("Pilih 2 atau lebih TextDraw di panel LAYERS atau di layar untuk menggunakan fitur grouping.");
            ImGui::Spacing();
            if (ImGui::Button("Pilih Semua TextDraw", ImVec2(-1, 36.0f * s))) {
                manager.select_all();
            }
        } else {
            // Group Move
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "1. Geser Grup (Move Together):");
            float b_size = 38.0f * s;
            ImGui::SetCursorPosX((w - b_size) * 0.5f - 8.0f * s);
            if (ImGui::Button("^##GUp", ImVec2(b_size, b_size))) {
                manager.move_selected(0.0f, -dpad_step, 0.0f);
            }
            ImGui::SetCursorPosX((w - b_size * 3) * 0.5f - 8.0f * s);
            if (ImGui::Button("<##GL", ImVec2(b_size, b_size))) {
                manager.move_selected(-dpad_step, 0.0f, 0.0f);
            }
            ImGui::SameLine();
            ImGui::Button(std::to_string((int)dpad_step).c_str(), ImVec2(b_size, b_size));
            ImGui::SameLine();
            if (ImGui::Button(">##GR", ImVec2(b_size, b_size))) {
                manager.move_selected(dpad_step, 0.0f, 0.0f);
            }
            ImGui::SetCursorPosX((w - b_size) * 0.5f - 8.0f * s);
            if (ImGui::Button("v##GDn", ImVec2(b_size, b_size))) {
                manager.move_selected(0.0f, dpad_step, 0.0f);
            }
            
            ImGui::Separator();
            
            // Alignment
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "2. Perataan Grup (Group Alignment):");
            ImVec2 ab_size(105.0f * s, 32.0f * s);
            if (ImGui::Button("Rata Kiri", ab_size)) manager.align_selected_left();
            ImGui::SameLine();
            if (ImGui::Button("Rata Tengah H", ab_size)) manager.align_selected_center_h();
            ImGui::SameLine();
            if (ImGui::Button("Rata Kanan", ab_size)) manager.align_selected_right();
            
            if (ImGui::Button("Rata Atas", ab_size)) manager.align_selected_top();
            ImGui::SameLine();
            if (ImGui::Button("Rata Tengah V", ab_size)) manager.align_selected_center_v();
            ImGui::SameLine();
            if (ImGui::Button("Rata Bawah", ab_size)) manager.align_selected_bottom();
            
            ImGui::Separator();
            
            // Group Colors
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "3. Warna Grup Serentak:");
            static float grp_col[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            ImGui::ColorEdit4("##GrpColorPicker", grp_col);
            uint32_t col_uint = ((uint32_t)(grp_col[0] * 255.0f) << 24) |
                                ((uint32_t)(grp_col[1] * 255.0f) << 16) |
                                ((uint32_t)(grp_col[2] * 255.0f) << 8)  |
                                ((uint32_t)(grp_col[3] * 255.0f));
            if (ImGui::Button("Terapkan ke Text Color", ImVec2(145.0f * s, 30.0f * s))) {
                manager.set_selected_color(col_uint, 0);
            }
            ImGui::SameLine();
            if (ImGui::Button("Terapkan ke Box Color", ImVec2(145.0f * s, 30.0f * s))) {
                manager.set_selected_color(col_uint, 1);
            }
            ImGui::SameLine();
            if (ImGui::Button("Terapkan ke Shadow Col", ImVec2(145.0f * s, 30.0f * s))) {
                manager.set_selected_color(col_uint, 2);
            }
            
            ImGui::Separator();
            
            // Group Actions
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "4. Operasi Batch:");
            if (ImGui::Button("Auto-fit Ukuran Teks Semua", ImVec2(180.0f * s, 32.0f * s))) {
                manager.auto_fit_selected_text_size();
            }
            ImGui::SameLine();
            if (ImGui::Button("Duplikasi Grup", ImVec2(120.0f * s, 32.0f * s))) {
                manager.duplicate_selected();
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.20f, 0.20f, 0.90f));
            if (ImGui::Button("Hapus Grup", ImVec2(100.0f * s, 32.0f * s))) {
                manager.delete_selected();
            }
            ImGui::PopStyleColor();
        }
        
        ImGui::Separator();
        if (ImGui::Button("Tutup", ImVec2(100.0f * s, 32.0f * s))) {
            show_group_modal = false;
        }
    }
    ImGui::End();
}

void EditorUI::render_trash_modal(TextDrawManager& manager) {
    ImGuiIO& io = ImGui::GetIO();
    float s = std::clamp(ui_scale, 1.0f, 1.35f);
    float w = std::min(io.DisplaySize.x * 0.92f, 520.0f * s);
    float h = std::min(io.DisplaySize.y * 0.88f, 440.0f * s);
    
    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - w) * 0.5f, (io.DisplaySize.y - h) * 0.5f), ImGuiCond_Always);
    
    if (ImGui::Begin("Tong Sampah / Recycle Bin (Silinenler)", &show_trash_modal, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        const auto& trash = manager.get_trash();
        ImGui::Text("Jumlah TextDraw Terhapus: %zu", trash.size());
        ImGui::Separator();
        
        if (trash.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Tong sampah kosong. Tidak ada TextDraw yang baru saja dihapus.");
        } else {
            if (ImGui::Button("Pulihkan Semua (Restore All)", ImVec2(190.0f * s, 34.0f * s))) {
                manager.restore_all_deleted();
            }
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.20f, 0.20f, 0.90f));
            if (ImGui::Button("Kosongkan Sampah", ImVec2(140.0f * s, 34.0f * s))) {
                manager.empty_trash();
            }
            ImGui::PopStyleColor();
            
            ImGui::Separator();
            
            float list_h = h - 145.0f * s;
            if (ImGui::BeginChild("##TrashListChild", ImVec2(0, list_h), true)) {
                for (size_t i = 0; i < trash.size(); ++i) {
                    const auto& item = trash[i];
                    ImGui::PushID((int)i);
                    
                    std::string prev = item.textdraw.text.size() > 18 ? (item.textdraw.text.substr(0, 18) + "..") : item.textdraw.text;
                    ImGui::Text("[%s] %s (\"%s\")", item.time_str.c_str(), item.textdraw.variable_name.c_str(), prev.c_str());
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 85.0f * s);
                    
                    if (ImGui::Button("Pulihkan", ImVec2(80.0f * s, 26.0f * s))) {
                        manager.restore_deleted(i);
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopID();
                    ImGui::Separator();
                }
            }
            ImGui::EndChild();
        }
        
        ImGui::Separator();
        if (ImGui::Button("Tutup", ImVec2(100.0f * s, 32.0f * s))) {
            show_trash_modal = false;
        }
    }
    ImGui::End();
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

