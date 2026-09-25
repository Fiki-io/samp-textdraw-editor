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

void EditorUI::init() {
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

    style.WindowRounding    = 8.0f;
    style.ChildRounding     = 6.0f;
    style.FrameRounding     = 6.0f;
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 6.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.5f;
    style.ItemSpacing       = ImVec2(8.0f, 6.0f);
    style.TouchExtraPadding = ImVec2(4.0f, 4.0f); // Helpful for mobile touch
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
            // Real 3D Preview pass with DffRenderer
            DffRenderer::get().render_preview_model(
                td.preview_model, sx, sy, sw, sh,
                td.rot_x, td.rot_y, td.rot_z, td.zoom,
                td.veh_color1, td.veh_color2
            );
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
        ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.15f, 1.0f), "GTA SA-MP TEXTDRAW");
        ImGui::Separator();
        
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project")) manager.clear_all();
            ImGui::Separator();
            if (ImGui::MenuItem("Save Project (.json)")) open_save_project_modal();
            if (ImGui::MenuItem("Open Project (.json)")) open_load_project_modal();
            ImGui::Separator();
            if (ImGui::MenuItem("Export Pawn Code (.pwn)")) open_export_modal();
            if (ImGui::MenuItem("Import Pawn Code...")) open_import_modal();
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, manager.can_undo())) manager.undo();
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, manager.can_redo())) manager.redo();
            ImGui::Separator();
            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) manager.duplicate_selected();
            if (ImGui::MenuItem("Delete", "Del")) manager.delete_selected();
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            bool is_4_3 = (viewport.aspect_mode == AspectRatioMode::RATIO_4_3);
            if (ImGui::MenuItem("4:3 Classic SA-MP", nullptr, is_4_3)) {
                viewport.aspect_mode = AspectRatioMode::RATIO_4_3;
                viewport.update_viewport_rect();
            }
            if (ImGui::MenuItem("16:9 Widescreen", nullptr, !is_4_3)) {
                viewport.aspect_mode = AspectRatioMode::RATIO_16_9;
                viewport.update_viewport_rect();
            }
            ImGui::Separator();
            ImGui::MenuItem("Grid Lines", nullptr, &viewport.enable_grid);
            if (ImGui::MenuItem("Reset Zoom & Pan")) viewport.reset_view();
            ImGui::EndMenu();
        }
        
        // Quick tools in menu bar
        ImGui::Separator();
        if (ImGui::Button("Undo") && manager.can_undo()) manager.undo();
        if (ImGui::Button("Redo") && manager.can_redo()) manager.redo();
        
        ImGui::Separator();
        ImGui::Text("Snap:");
        ImGui::SetNextItemWidth(70);
        float steps[] = {0.1f, 0.5f, 1.0f, 5.0f, 10.0f};
        const char* step_names[] = {"0.1", "0.5", "1.0", "5.0", "10.0"};
        int cur_step = 2; // default 1.0
        for (int i = 0; i < 5; ++i) if (std::abs(viewport.grid_step - steps[i]) < 0.05f) cur_step = i;
        if (ImGui::Combo("##Snap", &cur_step, step_names, 5)) {
            viewport.grid_step = steps[cur_step];
            dpad_step = steps[cur_step];
        }
        
        // Export button right aligned
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 110.0f);
        if (ImGui::Button("EXPORT PWN")) {
            open_export_modal();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void EditorUI::render_bottom_toolbar(TextDrawManager& manager, Viewport& viewport) {
    ImGuiIO& io = ImGui::GetIO();
    float bar_w = std::min(io.DisplaySize.x - 40.0f, 620.0f);
    float bar_h = 56.0f;
    
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - bar_w) * 0.5f, io.DisplaySize.y - bar_h - 16.0f));
    ImGui::SetNextWindowSize(ImVec2(bar_w, bar_h));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
                             
    if (ImGui::Begin("##BottomToolbar", nullptr, flags)) {
        if (ImGui::Button("+ TEXT", ImVec2(90, 40))) {
            manager.create_text(320.0f, 240.0f, "New Textdraw");
        }
        ImGui::SameLine();
        if (ImGui::Button("+ BOX", ImVec2(90, 40))) {
            manager.create_box(320.0f, 240.0f, 140.0f, 40.0f, 0x000000A0);
        }
        ImGui::SameLine();
        if (ImGui::Button("+ SPRITE", ImVec2(100, 40))) {
            show_sprite_picker = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ 3D MODEL", ImVec2(110, 40))) {
            show_model_picker = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        
        bool has_sel = (manager.get_active_textdraw() != nullptr);
        if (!has_sel) ImGui::BeginDisabled();
        if (ImGui::Button("CLONE", ImVec2(80, 40))) {
            manager.duplicate_selected();
        }
        ImGui::SameLine();
        if (ImGui::Button("DEL", ImVec2(60, 40))) {
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
    float panel_w = 340.0f;
    float panel_h = io.DisplaySize.y - 120.0f;
    
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panel_w - 16.0f, 40.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panel_w, panel_h), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Properties Inspector", nullptr)) {
        // Variable Name & Player Mode
        char var_buf[64];
        strncpy(var_buf, td->variable_name.c_str(), sizeof(var_buf));
        if (ImGui::InputText("Variable", var_buf, sizeof(var_buf))) {
            td->variable_name = var_buf;
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
            // Text Content
            char text_buf[256];
            strncpy(text_buf, td->text.c_str(), sizeof(text_buf));
            if (ImGui::InputText("Text Content", text_buf, sizeof(text_buf))) {
                td->text = text_buf;
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
            if (ImGui::InputText("Sprite Name", sprite_buf, sizeof(sprite_buf))) {
                td->text = sprite_buf;
            }
            ImGui::SameLine();
            if (ImGui::Button("Browse##Sprite")) {
                show_sprite_picker = true;
            }
        } else if (td->font == 5) { // 3D Model Preview
            ImGui::InputInt("Model ID", &td->preview_model);
            ImGui::SameLine();
            if (ImGui::Button("Browse##Model")) {
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
    float w = 150.0f;
    float h = 150.0f;
    
    ImGui::SetNextWindowPos(ImVec2(16.0f, io.DisplaySize.y - h - 70.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(w, h));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
    if (ImGui::Begin("Micro D-Pad", nullptr, flags)) {
        float btn_size = 38.0f;
        
        // Up
        ImGui::SetCursorPosX((w - btn_size) * 0.5f - 8.0f);
        if (ImGui::Button("^##Up", ImVec2(btn_size, btn_size))) {
            manager.move_selected(0.0f, -dpad_step, 0.0f);
        }
        
        // Left, Step, Right
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
        if (ImGui::Button("<##Left", ImVec2(btn_size, btn_size))) {
            manager.move_selected(-dpad_step, 0.0f, 0.0f);
        }
        ImGui::SameLine();
        char step_lbl[16];
        snprintf(step_lbl, sizeof(step_lbl), "%.1f", dpad_step);
        if (ImGui::Button(step_lbl, ImVec2(btn_size + 4.0f, btn_size))) {
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
        ImGui::SetCursorPosX((w - btn_size) * 0.5f - 8.0f);
        if (ImGui::Button("v##Down", ImVec2(btn_size, btn_size))) {
            manager.move_selected(0.0f, dpad_step, 0.0f);
        }
    }
    ImGui::End();
}

void EditorUI::render_sprite_picker(TextDrawManager& manager) {
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Browse GTA SA Sprites (591 Sprites)", &show_sprite_picker)) {
        ImGui::InputText("Filter", sprite_search_filter, sizeof(sprite_search_filter));
        std::string filter = sprite_search_filter;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
        
        ImGui::Separator();
        
        const auto& groups = AssetManager::get().get_sprites();
        if (ImGui::BeginChild("SpriteList")) {
            for (const auto& [txd, list] : groups) {
                // Check if any sprite matches
                bool group_matches = filter.empty() || (txd.find(filter) != std::string::npos);
                if (!group_matches) {
                    for (const auto& s : list) {
                        if (s.name.find(filter) != std::string::npos) {
                            group_matches = true;
                            break;
                        }
                    }
                }
                if (!group_matches) continue;
                
                if (ImGui::CollapsingHeader(txd.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (const auto& s : list) {
                        if (!filter.empty() && s.full_name.find(filter) == std::string::npos) continue;
                        
                        if (ImGui::Button(s.full_name.c_str(), ImVec2(-1, 32))) {
                            TextDraw* td = manager.get_active_textdraw();
                            if (td) {
                                td->font = 4;
                                td->text = s.full_name;
                                td->text_width = (float)s.width;
                                td->text_height = (float)s.height;
                            } else {
                                manager.create_sprite(320.0f, 240.0f, (float)s.width, (float)s.height, s.full_name);
                            }
                            show_sprite_picker = false;
                        }
                    }
                }
            }
            ImGui::EndChild();
        }
    }
    ImGui::End();
}

void EditorUI::render_model_picker(TextDrawManager& manager) {
    ImGui::SetNextWindowSize(ImVec2(550, 480), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Browse 3D Preview Models", &show_model_picker)) {
        ImGui::InputText("Search Vehicles / Skins", model_search_filter, sizeof(model_search_filter));
        std::string filter = model_search_filter;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);
        
        ImGui::Separator();
        
        if (ImGui::BeginTabBar("ModelTabs")) {
            if (ImGui::BeginTabItem("Vehicles (400 - 611)")) {
                const auto& vehs = AssetManager::get().get_vehicles();
                for (const auto& v : vehs) {
                    std::string label = std::to_string(v.id) + ": " + v.name + " (" + v.type + ")";
                    std::string low = label;
                    std::transform(low.begin(), low.end(), low.begin(), ::tolower);
                    if (!filter.empty() && low.find(filter) == std::string::npos) continue;
                    
                    if (ImGui::Selectable(label.c_str())) {
                        TextDraw* td = manager.get_active_textdraw();
                        if (td) {
                            td->font = 5;
                            td->preview_model = v.id;
                            td->text = std::to_string(v.id);
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
                for (const auto& s : skins) {
                    std::string label = std::to_string(s.id) + ": " + s.name;
                    std::string low = label;
                    std::transform(low.begin(), low.end(), low.begin(), ::tolower);
                    if (!filter.empty() && low.find(filter) == std::string::npos) continue;
                    
                    if (ImGui::Selectable(label.c_str())) {
                        TextDraw* td = manager.get_active_textdraw();
                        if (td) {
                            td->font = 5;
                            td->preview_model = s.id;
                            td->text = std::to_string(s.id);
                        } else {
                            manager.create_preview_model(320.0f, 240.0f, 80.0f, 120.0f, s.id);
                        }
                        show_model_picker = false;
                    }
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void EditorUI::render_carcols_palette_modal(TextDraw* td, bool target_is_box, int veh_col_index) {
    if (!td) {
        show_carcols_picker = false;
        return;
    }
    ImGui::SetNextWindowSize(ImVec2(480, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("SA-MP Color Palette (256 Colors)", &show_carcols_picker)) {
        const auto& cols = AssetManager::get().get_colors();
        int cols_per_row = 8;
        float btn_w = 48.0f;
        float btn_h = 32.0f;
        
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
    ImGui::End();
}

void EditorUI::open_export_modal() {
    show_export_modal = true;
}

void EditorUI::render_export_modal(TextDrawManager& manager) {
    ImGui::SetNextWindowSize(ImVec2(650, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Export Pawn Code", &show_export_modal)) {
        if (export_code_buffer.empty() || ImGui::Button("Refresh Code")) {
            export_code_buffer = PawnExporter::export_pawn(manager.get_all_textdraws());
        }
        ImGui::SameLine();
        if (ImGui::Button("Copy to Clipboard")) {
            ImGui::SetClipboardText(export_code_buffer.c_str());
        }
        
        ImGui::Separator();
        
        ImGui::InputTextMultiline("##PawnOutput", const_cast<char*>(export_code_buffer.c_str()),
                                  export_code_buffer.size() + 1, ImVec2(-1, -1),
                                  ImGuiInputTextFlags_ReadOnly);
    }
    ImGui::End();
}

void EditorUI::open_import_modal() {
    show_import_modal = true;
}

void EditorUI::render_import_modal(TextDrawManager& manager) {
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Import Pawn Code", &show_import_modal)) {
        ImGui::Text("Paste your TextDraw Pawn code below:");
        ImGui::InputTextMultiline("##PawnImport", import_buffer, sizeof(import_buffer), ImVec2(-1, -40));
        
        if (ImGui::Button("Import Script", ImVec2(120, 32))) {
            // Very simple parser for TextDrawCreate / CreatePlayerTextDraw
            std::stringstream ss(import_buffer);
            std::string line;
            while (std::getline(ss, line)) {
                size_t p = line.find("TextDrawCreate(");
                if (p != std::string::npos) {
                    float x = 320.0f, y = 240.0f;
                    char txt[128] = "";
                    if (sscanf(line.c_str() + p + 15, "%f, %f, \"%[^\"]\"", &x, &y, txt) >= 2) {
                        manager.create_text(x, y, txt);
                    }
                }
            }
            show_import_modal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 32))) {
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
    ImGui::SetNextWindowSize(ImVec2(480, 220), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Save Project (.json)", &show_save_project_modal)) {
        ImGui::Text("Enter Project Name:");
        ImGui::InputText(".json##Filename", project_name_buf, sizeof(project_name_buf));
        
        ImGui::Separator();
        if (ImGui::Button("Save Project", ImVec2(130, 36))) {
            std::string file_name = std::string(project_name_buf) + ".json";
            std::string full_path = storage_path.empty() ? file_name : (storage_path + "/" + file_name);
            manager.save_project_to_file(full_path, project_name_buf);
            show_save_project_modal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 36))) {
            show_save_project_modal = false;
        }
    }
    ImGui::End();
}

void EditorUI::render_load_project_modal(TextDrawManager& manager) {
    ImGui::SetNextWindowSize(ImVec2(520, 420), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Open Project (.json)", &show_load_project_modal)) {
        ImGui::Text("Saved Projects in Storage:");
        ImGui::Separator();
        
        DIR* dir = opendir(storage_path.empty() ? "." : storage_path.c_str());
        if (dir) {
            struct dirent* ent;
            bool found_any = false;
            while ((ent = readdir(dir)) != nullptr) {
                std::string fname = ent->d_name;
                if (fname.size() > 5 && fname.substr(fname.size() - 5) == ".json") {
                    found_any = true;
                    if (ImGui::Selectable(fname.c_str(), false, ImGuiSelectableFlags_None, ImVec2(-1, 32))) {
                        std::string full_path = storage_path.empty() ? fname : (storage_path + "/" + fname);
                        manager.load_project_from_file(full_path);
                        show_load_project_modal = false;
                        break;
                    }
                }
            }
            closedir(dir);
            if (!found_any) {
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No saved .json projects found yet.");
            }
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "Could not access projects directory.");
        }
        
        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(100, 32))) {
            show_load_project_modal = false;
        }
    }
    ImGui::End();
}

