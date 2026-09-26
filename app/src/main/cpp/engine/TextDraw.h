#pragma once

#include <string>
#include <vector>
#include <cstdint>

enum class TextDrawType {
    TEXT = 0,       // Font 0, 1, 2, 3
    SPRITE = 4,     // Font 4 (TXD:TEXTURE)
    MODEL_PREVIEW = 5 // Font 5 (3D Model DFF)
};

enum class TextDrawAlignment {
    LEFT = 1,
    CENTER = 2,
    RIGHT = 3
};

struct TextDraw {
    int id = 0;
    std::string variable_name = "Textdraw0";
    bool is_player = false; // false = Global (Text:), true = PlayerText:
    
    // Core Coordinates (SA-MP 640x480 virtual canvas)
    float x = 320.0f;
    float y = 240.0f;
    
    // Content
    std::string text = "New Textdraw";
    
    // Font & Styling
    int font = 1; // 0=Gothic, 1=Standard, 2=Subheader, 3=Pricedown, 4=Sprite, 5=Model Preview
    float letter_width = 0.48f;
    float letter_height = 1.8f;
    
    // TextSize (also bounding box for clickable / box / model preview bounds)
    float text_width = 120.0f;
    float text_height = 20.0f;
    
    TextDrawAlignment alignment = TextDrawAlignment::LEFT;
    uint32_t color = 0xFFFFFFFF; // RGBA
    
    // Box
    bool use_box = false;
    uint32_t box_color = 0x00000080; // Black semi-transparent
    
    // Shadow & Outline
    int shadow = 1;
    int outline = 0;
    uint32_t background_color = 0x000000FF; // Shadow/outline color
    
    bool proportional = true;
    bool selectable = false;
    
    // Preview Model (Font 5)
    int preview_model = 411; // Default: Infernus
    float rot_x = 0.0f;
    float rot_y = 0.0f;
    float rot_z = 0.0f;
    float zoom = 1.0f;
    int veh_color1 = 1; // White
    int veh_color2 = 1;
    
    // Editor State
    bool is_selected = false;
    bool is_visible = true;
    bool is_locked = false;
    int z_index = 0;
    
    // Grouping
    int group_id = 0;
    bool is_grouped = false;

    // Auto-calculate TextSize for Font 0-3 clickable textdraws
    void auto_calculate_text_size() {
        if (font >= 0 && font <= 3) {
            int char_count = 0;
            for (size_t i = 0; i < text.size(); ++i) {
                if (text[i] == '~' && i + 2 < text.size() && text[i + 2] == '~') {
                    i += 2;
                    continue;
                }
                char_count++;
            }
            if (char_count < 1) char_count = 1;
            float est_w = std::max(12.0f, (float)char_count * (letter_width * 20.0f));
            float est_h = std::max(8.0f, letter_height * 20.0f);
            // SA-MP: TextDrawTextSize for LEFT = absolute bottom-right corner
            if (alignment == TextDrawAlignment::LEFT) {
                text_width  = x + est_w;
                text_height = y + est_h;
            } else if (alignment == TextDrawAlignment::CENTER) {
                // For center: X = total width, Y = unused (set to height)
                text_width  = est_w;
                text_height = est_h;
            } else {
                // For RIGHT: textsize = top-left corner of box
                text_width  = x - est_w;
                text_height = y;
            }
        }
    }

    // Helper: compute screen-space bounding box (in SA-MP 640x480 coords)
    // for rendering boxes, selections, and hit-testing.
    // Matches SA-MP TextDrawTextSize semantics:
    //   LEFT  : textsize = (right, bottom) absolute coords
    //   CENTER: x = center, textsize.x = box width, textsize.y = box height
    //   RIGHT : textsize = (left, top) of box, x = right edge
    //   Font4/5 : textsize = (width_offset, height_offset) from origin
    void get_bounds(float& out_x1, float& out_y1, float& out_x2, float& out_y2) const {
        if (font == 4 || font == 5) {
            // Sprite / 3D model: textsize = width/height offset
            out_x1 = x;
            out_y1 = y;
            out_x2 = x + text_width;
            out_y2 = y + text_height;
        } else if (use_box && text_width > 0.0f && text_height > 0.0f) {
            // Box mode: textsize semantics per alignment
            if (alignment == TextDrawAlignment::CENTER) {
                // x = center, textsize.x = total width
                float hw = text_width * 0.5f;
                out_x1 = x - hw;
                out_y1 = y;
                out_x2 = x + hw;
                out_y2 = y + text_height;
            } else if (alignment == TextDrawAlignment::RIGHT) {
                // textsize = (left, top), x = right edge
                out_x1 = text_width;
                out_y1 = text_height;
                out_x2 = x;
                out_y2 = text_height + (x - text_width) * 0.3f; // approx height
                // Prefer direct: textsize.(x,y) is top-left, x is right
                out_x1 = text_width;
                out_y1 = y;
                out_x2 = x;
                out_y2 = text_height;
            } else {
                // LEFT: textsize = (right, bottom) absolute coords
                out_x1 = x;
                out_y1 = y;
                out_x2 = text_width;
                out_y2 = text_height;
            }
        } else {
            // No box: estimate from letter size and text length
            int char_count = 0;
            for (size_t i = 0; i < text.size(); ++i) {
                if (text[i] == '~' && i + 2 < text.size() && text[i + 2] == '~') {
                    i += 2;
                    continue;
                }
                char_count++;
            }
            if (char_count < 1) char_count = 1;
            float w = (float)char_count * (letter_width * 20.0f);
            float h = letter_height * 20.0f;
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
};
