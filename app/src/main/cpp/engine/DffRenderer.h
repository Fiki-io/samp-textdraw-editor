#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <GLES3/gl3.h>

struct DffVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct DffMesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei index_count = 0;
    float bound_sphere[4] = {0.0f, 0.0f, 0.0f, 1.0f}; // x, y, z, radius
};

class DffRenderer {
public:
    static DffRenderer& get();
    
    void init();
    void begin_frame();
    GLuint render_to_texture(int model_id, float rot_x, float rot_y, float rot_z, float zoom,
                             int veh_col1, int veh_col2);

private:
    DffRenderer() = default;
    ~DffRenderer();
    
    GLuint shader_program = 0;
    GLint u_mvp_loc = -1;
    GLint u_model_loc = -1;
    GLint u_color1_loc = -1;
    GLint u_color2_loc = -1;
    GLint u_light_dir_loc = -1;

    static const int FBO_COUNT = 16;
    static const int FBO_SIZE = 256;
    GLuint fbos[FBO_COUNT] = {0};
    GLuint fbo_textures[FBO_COUNT] = {0};
    GLuint fbo_depths[FBO_COUNT] = {0};
    int current_fbo_index = 0;
    
    std::unordered_map<int, DffMesh> mesh_cache;
    
    bool load_model_mesh(int model_id, DffMesh& out_mesh);
    bool parse_dff_data(const std::vector<uint8_t>& data, DffMesh& out_mesh);
    void create_fallback_mesh(DffMesh& out_mesh);
};
