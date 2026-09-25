#include "Viewport.h"
#include <algorithm>

Viewport::Viewport() {
    reset_view();
}

void Viewport::set_screen_size(int width, int height) {
    screen_width = width;
    screen_height = height;
    update_viewport_rect();
}

void Viewport::update_viewport_rect() {
    float target_aspect = (aspect_mode == AspectRatioMode::RATIO_4_3) ? (4.0f / 3.0f) : (16.0f / 9.0f);
    float screen_aspect = (float)screen_width / (float)screen_height;
    
    // Fit 640x480 inside screen maintaining target aspect ratio
    float base_w, base_h;
    if (screen_aspect > target_aspect) {
        base_h = (float)screen_height * 0.92f;
        base_w = base_h * target_aspect;
    } else {
        base_w = (float)screen_width * 0.92f;
        base_h = base_w / target_aspect;
    }
    
    // Apply zoom
    canvas_screen_w = base_w * zoom;
    canvas_screen_h = base_h * zoom;
    
    // Center on screen + pan offset
    canvas_screen_x = ((float)screen_width - canvas_screen_w) * 0.5f + pan_x;
    canvas_screen_y = ((float)screen_height - canvas_screen_h) * 0.5f + pan_y;
}

void Viewport::samp_to_screen(float samp_x, float samp_y, float& out_sx, float& out_sy) const {
    float norm_x = samp_x / 640.0f;
    float norm_y = samp_y / 480.0f;
    out_sx = canvas_screen_x + norm_x * canvas_screen_w;
    out_sy = canvas_screen_y + norm_y * canvas_screen_h;
}

void Viewport::screen_to_samp(float sx, float sy, float& out_samp_x, float& out_samp_y) const {
    if (canvas_screen_w <= 0.001f || canvas_screen_h <= 0.001f) {
        out_samp_x = 0.0f;
        out_samp_y = 0.0f;
        return;
    }
    float norm_x = (sx - canvas_screen_x) / canvas_screen_w;
    float norm_y = (sy - canvas_screen_y) / canvas_screen_h;
    out_samp_x = norm_x * 640.0f;
    out_samp_y = norm_y * 480.0f;
}

float Viewport::samp_to_screen_scale_x(float samp_w) const {
    return (samp_w / 640.0f) * canvas_screen_w;
}

float Viewport::samp_to_screen_scale_y(float samp_h) const {
    return (samp_h / 480.0f) * canvas_screen_h;
}

void Viewport::set_zoom(float z) {
    zoom = std::clamp(z, 0.25f, 10.0f);
    update_viewport_rect();
}

void Viewport::add_pan(float delta_x, float delta_y) {
    pan_x += delta_x;
    pan_y += delta_y;
    update_viewport_rect();
}

void Viewport::reset_view() {
    zoom = 1.0f;
    pan_x = 0.0f;
    pan_y = 0.0f;
    update_viewport_rect();
}

float Viewport::snap_val(float val, float step) {
    if (step <= 0.001f) return val;
    return std::round(val / step) * step;
}
