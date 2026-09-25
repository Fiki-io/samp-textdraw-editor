#include "DffRenderer.h"
#include "AssetManager.h"
#include <android/log.h>
#include <cmath>
#include <cstring>

#define LOG_TAG "TextDraw_DffRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static const char* kVertexShader = R"(#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 vNormal;
out vec2 vUV;
out vec3 vFragPos;

void main() {
    vNormal = mat3(uModel) * aNormal;
    vUV = aUV;
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

static const char* kFragmentShader = R"(#version 300 es
precision mediump float;

in vec3 vNormal;
in vec2 vUV;
in vec3 vFragPos;

uniform vec3 uColor1;
uniform vec3 uColor2;
uniform vec3 uLightDir;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    float diff = max(dot(norm, lightDir), 0.25);
    
    // GTA car shader lighting
    vec3 baseColor = mix(uColor1, uColor2, step(0.5, vUV.y));
    if (length(uColor1) < 0.01 && length(uColor2) < 0.01) {
        baseColor = vec3(0.85, 0.85, 0.9);
    }
    
    vec3 ambient = vec3(0.35, 0.35, 0.4);
    vec3 result = (ambient + diff * vec3(1.0, 0.98, 0.92)) * baseColor;
    FragColor = vec4(result, 1.0);
}
)";

// Simple 4x4 matrix helpers
struct Mat4 {
    float m[16];
    
    static Mat4 identity() {
        Mat4 r = {0};
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }
    
    static Mat4 perspective(float fov_rad, float aspect, float near_z, float far_z) {
        Mat4 r = {0};
        float tan_half = std::tan(fov_rad / 2.0f);
        r.m[0] = 1.0f / (aspect * tan_half);
        r.m[5] = 1.0f / tan_half;
        r.m[10] = -(far_z + near_z) / (far_z - near_z);
        r.m[11] = -1.0f;
        r.m[14] = -(2.0f * far_z * near_z) / (far_z - near_z);
        return r;
    }
    
    static Mat4 multiply(const Mat4& a, const Mat4& b) {
        Mat4 r = {0};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    r.m[j * 4 + i] += a.m[k * 4 + i] * b.m[j * 4 + k];
                }
            }
        }
        return r;
    }
    
    static Mat4 rotation_zyx(float rx, float ry, float rz) {
        float cx = std::cos(rx), sx = std::sin(rx);
        float cy = std::cos(ry), sy = std::sin(ry);
        float cz = std::cos(rz), sz = std::sin(rz);
        
        Mat4 r = identity();
        r.m[0] = cy * cz;
        r.m[1] = cy * sz;
        r.m[2] = -sy;
        
        r.m[4] = sx * sy * cz - cx * sz;
        r.m[5] = sx * sy * sz + cx * cz;
        r.m[6] = sx * cy;
        
        r.m[8] = cx * sy * cz + sx * sz;
        r.m[9] = cx * sy * sz - sx * cz;
        r.m[10] = cx * cy;
        return r;
    }
    
    static Mat4 translation(float tx, float ty, float tz) {
        Mat4 r = identity();
        r.m[12] = tx;
        r.m[13] = ty;
        r.m[14] = tz;
        return r;
    }
};

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOGE("Shader compilation error: %s", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

DffRenderer& DffRenderer::get() {
    static DffRenderer instance;
    return instance;
}

DffRenderer::~DffRenderer() {
    if (shader_program) glDeleteProgram(shader_program);
    for (auto& pair : mesh_cache) {
        if (pair.second.vao) glDeleteVertexArrays(1, &pair.second.vao);
        if (pair.second.vbo) glDeleteBuffers(1, &pair.second.vbo);
        if (pair.second.ebo) glDeleteBuffers(1, &pair.second.ebo);
    }
    mesh_cache.clear();
}

void DffRenderer::init() {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, kVertexShader);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (!vs || !fs) return;
    
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vs);
    glAttachShader(shader_program, fs);
    glLinkProgram(shader_program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    u_mvp_loc = glGetUniformLocation(shader_program, "uMVP");
    u_model_loc = glGetUniformLocation(shader_program, "uModel");
    u_color1_loc = glGetUniformLocation(shader_program, "uColor1");
    u_color2_loc = glGetUniformLocation(shader_program, "uColor2");
    u_light_dir_loc = glGetUniformLocation(shader_program, "uLightDir");
    LOGI("DffRenderer initialized successfully.");
}

bool DffRenderer::load_model_mesh(int model_id, DffMesh& out_mesh) {
    auto it = mesh_cache.find(model_id);
    if (it != mesh_cache.end()) {
        out_mesh = it->second;
        return true;
    }
    
    // Find model name from database
    std::string dff_name = "";
    if (model_id >= 400 && model_id <= 611) {
        const auto& vehtar = AssetManager::get().get_vehicles();
        for (const auto& v : vehtar) {
            if (v.id == model_id) {
                dff_name = v.dff;
                break;
            }
        }
    } else {
        const auto& skintar = AssetManager::get().get_skins();
        for (const auto& s : skintar) {
            if (s.id == model_id) {
                dff_name = s.dff;
                break;
            }
        }
    }
    
    bool loaded = false;
    if (!dff_name.empty()) {
        auto bytes = AssetManager::get().load_model_dff(dff_name);
        if (!bytes.empty()) {
            loaded = parse_dff_data(bytes, out_mesh);
        }
    }
    
    if (!loaded) {
        create_fallback_mesh(out_mesh);
    }
    
    mesh_cache[model_id] = out_mesh;
    return true;
}

bool DffRenderer::parse_dff_data(const std::vector<uint8_t>& data, DffMesh& out_mesh) {
    if (data.size() < 64) return false;
    
    // Search for Geometry Struct Chunk (0x0F then 0x01)
    const uint8_t* p = data.data();
    size_t size = data.size();
    size_t offset = 0;
    
    while (offset + 12 <= size) {
        uint32_t chunk_type = *reinterpret_cast<const uint32_t*>(p + offset);
        uint32_t chunk_size = *reinterpret_cast<const uint32_t*>(p + offset + 4);
        
        if (chunk_type == 0x0F) { // rwID_GEOMETRY
            size_t geom_offset = offset + 12;
            if (geom_offset + 12 <= size) {
                uint32_t s_type = *reinterpret_cast<const uint32_t*>(p + geom_offset);
                uint32_t s_size = *reinterpret_cast<const uint32_t*>(p + geom_offset + 4);
                if (s_type == 0x01 && geom_offset + 12 + s_size <= size) {
                    const uint8_t* struct_ptr = p + geom_offset + 12;
                    uint16_t format_flags = *reinterpret_cast<const uint16_t*>(struct_ptr);
                    uint32_t num_tris = *reinterpret_cast<const uint32_t*>(struct_ptr + 4);
                    uint32_t num_verts = *reinterpret_cast<const uint32_t*>(struct_ptr + 8);
                    
                    if (num_verts > 0 && num_tris > 0 && num_verts < 65536) {
                        size_t cur = 16;
                        if (format_flags & 0x0008) cur += num_verts * 4; // Prelit colors
                        const float* uvs = nullptr;
                        if (format_flags & 0x0004) { // Textured
                            uvs = reinterpret_cast<const float*>(struct_ptr + cur);
                            cur += num_verts * 8;
                        }
                        
                        // Triangles: (v2, v1, flags, v3)
                        const uint16_t* tri_raw = reinterpret_cast<const uint16_t*>(struct_ptr + cur);
                        cur += num_tris * 8;
                        
                        // Bounding sphere
                        const float* sphere = reinterpret_cast<const float*>(struct_ptr + cur);
                        out_mesh.bound_sphere[0] = sphere[0];
                        out_mesh.bound_sphere[1] = sphere[1];
                        out_mesh.bound_sphere[2] = sphere[2];
                        out_mesh.bound_sphere[3] = sphere[3];
                        cur += 16;
                        
                        uint32_t has_verts = *reinterpret_cast<const uint32_t*>(struct_ptr + cur); cur += 4;
                        uint32_t has_norms = *reinterpret_cast<const uint32_t*>(struct_ptr + cur); cur += 4;
                        
                        const float* verts = nullptr;
                        if (has_verts) {
                            verts = reinterpret_cast<const float*>(struct_ptr + cur);
                            cur += num_verts * 12;
                        }
                        const float* norms = nullptr;
                        if (has_norms) {
                            norms = reinterpret_cast<const float*>(struct_ptr + cur);
                            cur += num_verts * 12;
                        }
                        
                        if (verts) {
                            std::vector<DffVertex> vert_list(num_verts);
                            for (uint32_t v = 0; v < num_verts; ++v) {
                                vert_list[v].x = verts[v * 3 + 0];
                                vert_list[v].y = verts[v * 3 + 1];
                                vert_list[v].z = verts[v * 3 + 2];
                                if (norms) {
                                    vert_list[v].nx = norms[v * 3 + 0];
                                    vert_list[v].ny = norms[v * 3 + 1];
                                    vert_list[v].nz = norms[v * 3 + 2];
                                } else {
                                    vert_list[v].nx = 0.0f; vert_list[v].ny = 1.0f; vert_list[v].nz = 0.0f;
                                }
                                if (uvs) {
                                    vert_list[v].u = uvs[v * 2 + 0];
                                    vert_list[v].v = uvs[v * 2 + 1];
                                } else {
                                    vert_list[v].u = 0.0f; vert_list[v].v = 0.0f;
                                }
                            }
                            
                            std::vector<uint16_t> indices;
                            indices.reserve(num_tris * 3);
                            for (uint32_t t = 0; t < num_tris; ++t) {
                                uint16_t v2 = tri_raw[t * 4 + 0];
                                uint16_t v1 = tri_raw[t * 4 + 1];
                                uint16_t v3 = tri_raw[t * 4 + 3];
                                if (v1 < num_verts && v2 < num_verts && v3 < num_verts) {
                                    indices.push_back(v1);
                                    indices.push_back(v2);
                                    indices.push_back(v3);
                                }
                            }
                            
                            // Upload to OpenGL ES
                            glGenVertexArrays(1, &out_mesh.vao);
                            glGenBuffers(1, &out_mesh.vbo);
                            glGenBuffers(1, &out_mesh.ebo);
                            
                            glBindVertexArray(out_mesh.vao);
                            glBindBuffer(GL_ARRAY_BUFFER, out_mesh.vbo);
                            glBufferData(GL_ARRAY_BUFFER, vert_list.size() * sizeof(DffVertex), vert_list.data(), GL_STATIC_DRAW);
                            
                            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh.ebo);
                            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
                            
                            // Pos
                            glEnableVertexAttribArray(0);
                            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DffVertex), (void*)offsetof(DffVertex, x));
                            // Normal
                            glEnableVertexAttribArray(1);
                            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DffVertex), (void*)offsetof(DffVertex, nx));
                            // UV
                            glEnableVertexAttribArray(2);
                            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(DffVertex), (void*)offsetof(DffVertex, u));
                            
                            glBindVertexArray(0);
                            out_mesh.index_count = (GLsizei)indices.size();
                            return true;
                        }
                    }
                }
            }
        }
        offset += 12 + chunk_size;
    }
    return false;
}

void DffRenderer::create_fallback_mesh(DffMesh& out_mesh) {
    // Elegant low-poly vehicle prism fallback
    std::vector<DffVertex> verts = {
        // Front
        {-1.0f, -0.5f,  1.8f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
        { 1.0f, -0.5f,  1.8f,  0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        { 0.8f,  0.5f,  1.0f,  0.0f, 0.7f, 0.7f, 1.0f, 1.0f},
        {-0.8f,  0.5f,  1.0f,  0.0f, 0.7f, 0.7f, 0.0f, 1.0f},
        // Back
        {-1.0f, -0.5f, -1.8f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f},
        { 1.0f, -0.5f, -1.8f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f},
        { 0.8f,  0.6f, -1.0f,  0.0f, 0.7f, -0.7f, 1.0f, 1.0f},
        {-0.8f,  0.6f, -1.0f,  0.0f, 0.7f, -0.7f, 0.0f, 1.0f},
    };
    std::vector<uint16_t> indices = {
        0, 1, 2,  0, 2, 3, // Front
        5, 4, 7,  5, 7, 6, // Back
        3, 2, 6,  3, 6, 7, // Roof
        4, 5, 1,  4, 1, 0, // Bottom
        4, 0, 3,  4, 3, 7, // Left
        1, 5, 6,  1, 6, 2  // Right
    };
    
    out_mesh.bound_sphere[0] = 0.0f;
    out_mesh.bound_sphere[1] = 0.0f;
    out_mesh.bound_sphere[2] = 0.0f;
    out_mesh.bound_sphere[3] = 2.2f;
    
    glGenVertexArrays(1, &out_mesh.vao);
    glGenBuffers(1, &out_mesh.vbo);
    glGenBuffers(1, &out_mesh.ebo);
    
    glBindVertexArray(out_mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, out_mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(DffVertex), verts.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out_mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DffVertex), (void*)offsetof(DffVertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DffVertex), (void*)offsetof(DffVertex, nx));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(DffVertex), (void*)offsetof(DffVertex, u));
    
    glBindVertexArray(0);
    out_mesh.index_count = (GLsizei)indices.size();
}

void DffRenderer::render_preview_model(int model_id, float screen_x, float screen_y, float screen_w, float screen_h,
                                     float rot_x, float rot_y, float rot_z, float zoom,
                                     int veh_col1, int veh_col2) {
    if (!shader_program || screen_w <= 1.0f || screen_h <= 1.0f) return;
    
    DffMesh mesh;
    load_model_mesh(model_id, mesh);
    if (!mesh.vao || mesh.index_count == 0) return;
    
    // Set Scissor and Viewport for the textdraw preview box
    GLint orig_viewport[4];
    glGetIntegerv(GL_VIEWPORT, orig_viewport);
    GLboolean orig_scissor = glIsEnabled(GL_SCISSOR_TEST);
    GLint orig_scissor_box[4];
    glGetIntegerv(GL_SCISSOR_BOX, orig_scissor_box);
    
    // Invert Y for OpenGL coordinate space
    float gl_y = (float)orig_viewport[3] - (screen_y + screen_h);
    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint)screen_x, (GLint)gl_y, (GLsizei)screen_w, (GLsizei)screen_h);
    glViewport((GLint)screen_x, (GLint)gl_y, (GLsizei)screen_w, (GLsizei)screen_h);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glUseProgram(shader_program);
    
    // Matrices
    float aspect = screen_w / screen_h;
    Mat4 proj = Mat4::perspective(45.0f * (3.14159265f / 180.0f), aspect, 0.1f, 100.0f);
    
    float radius = mesh.bound_sphere[3];
    if (radius <= 0.1f) radius = 2.0f;
    float dist = (radius * 2.5f) / std::max(zoom, 0.1f);
    
    Mat4 view = Mat4::translation(0.0f, 0.0f, -dist);
    
    // SA-MP rotation: RotX, RotY, RotZ in degrees
    const float to_rad = 3.14159265f / 180.0f;
    Mat4 rot = Mat4::rotation_zyx(rot_x * to_rad, rot_y * to_rad, rot_z * to_rad);
    Mat4 center = Mat4::translation(-mesh.bound_sphere[0], -mesh.bound_sphere[1], -mesh.bound_sphere[2]);
    
    Mat4 model = Mat4::multiply(rot, center);
    Mat4 mv = Mat4::multiply(view, model);
    Mat4 mvp = Mat4::multiply(proj, mv);
    
    glUniformMatrix4fv(u_mvp_loc, 1, GL_FALSE, mvp.m);
    glUniformMatrix4fv(u_model_loc, 1, GL_FALSE, model.m);
    
    // Colors from Carcols
    const auto& carcols = AssetManager::get().get_colors();
    auto get_col_vec = [&](int cid) -> std::pair<float, std::pair<float, float>> {
        if (cid >= 0 && cid < (int)carcols.size()) {
            return {carcols[cid].r / 255.0f, {carcols[cid].g / 255.0f, carcols[cid].b / 255.0f}};
        }
        return {0.9f, {0.9f, 0.9f}};
    };
    
    auto c1 = get_col_vec(veh_col1);
    auto c2 = get_col_vec(veh_col2);
    glUniform3f(u_color1_loc, c1.first, c1.second.first, c1.second.second);
    glUniform3f(u_color2_loc, c2.first, c2.second.first, c2.second.second);
    glUniform3f(u_light_dir_loc, 0.5f, 0.8f, 0.6f);
    
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
    
    // Restore states
    glDisable(GL_DEPTH_TEST);
    glViewport(orig_viewport[0], orig_viewport[1], orig_viewport[2], orig_viewport[3]);
    if (orig_scissor) {
        glScissor(orig_scissor_box[0], orig_scissor_box[1], orig_scissor_box[2], orig_scissor_box[3]);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}
