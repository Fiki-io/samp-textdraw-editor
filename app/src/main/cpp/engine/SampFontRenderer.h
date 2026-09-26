#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "../imgui/imgui.h"
#include "TextDraw.h"

class Viewport;

struct SampGlyphMetric {
    uint8_t min_x;
    uint8_t max_x;
    uint8_t min_y;
    uint8_t max_y;
    uint8_t width;
};

class SampFontRenderer {
public:
    static SampFontRenderer& get();

    // Renders TextDraw text onto the ImGui draw list using authentic GTA SA font textures
    void render_textdraw(
        int font_id,
        const std::string& text,
        float samp_x,
        float samp_y,
        float letter_w,
        float letter_h,
        uint32_t text_color,
        uint32_t bg_color,
        int shadow,
        int outline,
        bool proportional,
        TextDrawAlignment alignment,
        const Viewport& viewport,
        ImDrawList* draw_list
    );

    // Measures text size in SA-MP 640x480 coordinate units
    ImVec2 measure_text_samp(
        int font_id,
        const std::string& text,
        float letter_w,
        float letter_h,
        bool proportional
    );

private:
    SampFontRenderer() = default;
    
    const SampGlyphMetric* get_metric(int font_id, char c) const;
    float get_char_width_samp(int font_id, char c, float letter_w, bool proportional) const;
    float get_space_width_samp(int font_id, float letter_w, bool proportional) const;
};
