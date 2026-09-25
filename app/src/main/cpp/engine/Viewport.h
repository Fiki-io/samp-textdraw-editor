#pragma once

#include <cmath>

enum class AspectRatioMode {
    RATIO_4_3,   // Classic SA-MP 4:3 Letterbox
    RATIO_16_9   // Widescreen 16:9
};

class Viewport {
public:
    Viewport();
    
    void set_screen_size(int width, int height);
    void update_viewport_rect();
    
    // Coordinate conversions
    void samp_to_screen(float samp_x, float samp_y, float& out_sx, float& out_sy) const;
    void screen_to_samp(float sx, float sy, float& out_samp_x, float& out_samp_y) const;
    
    float samp_to_screen_scale_x(float samp_w) const;
    float samp_to_screen_scale_y(float samp_h) const;
    
    // Zoom and Pan
    void set_zoom(float z);
    float get_zoom() const { return zoom; }
    void add_pan(float delta_x, float delta_y);
    void reset_view();
    
    // Snapping
    static float snap_val(float val, float step);
    
    // Canvas bounds on screen
    float canvas_screen_x = 0.0f;
    float canvas_screen_y = 0.0f;
    float canvas_screen_w = 0.0f;
    float canvas_screen_h = 0.0f;
    
    int screen_width = 1920;
    int screen_height = 1080;
    
    AspectRatioMode aspect_mode = AspectRatioMode::RATIO_4_3;
    bool enable_grid = true;
    float grid_step = 1.0f; // 0.1, 0.5, 1.0, 5.0, 10.0
    
    float pan_x = 0.0f;
    float pan_y = 0.0f;
    float zoom = 1.0f;
};
