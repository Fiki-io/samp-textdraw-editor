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
out vec3 vModelPos;
out vec3 vModelNormal;

void main() {
    vNormal = mat3(uModel) * aNormal;
    vModelNormal = aNormal;
    vUV = aUV;
    vModelPos = aPos;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

static const char* kFragmentShader = R"(#version 300 es
precision mediump float;

in vec3 vNormal;
in vec2 vUV;
in vec3 vModelPos;
in vec3 vModelNormal;

uniform vec3 uColor1;
uniform vec3 uColor2;
uniform vec3 uLightDir;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    
    // Key directional light
    float diff1 = max(dot(norm, lightDir), 0.0);
    // Fill light from opposite side
    vec3 fillDir = normalize(vec3(-lightDir.x, 0.5, -lightDir.z));
    float diff2 = max(dot(norm, fillDir), 0.0) * 0.35;
    
    // Specular highlight
    vec3 viewDir = vec3(0.0, 0.0, 1.0);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), 20.0) * 0.35;
    
    // Base vehicle paint colors
    vec3 bodyCol = uColor1;
    if (length(bodyCol) < 0.02) {
        bodyCol = vec3(0.85, 0.15, 0.15); // Default attractive sports red if black
    }
    
    vec3 matColor = bodyCol;
    if (vUV.y > 0.55 && length(uColor2) > 0.02) {
        matColor = mix(bodyCol, uColor2, 0.85);
    }
    
    // Tires & undercarriage detection (low Y in OpenGL coordinates)
    if (vModelPos.y < -0.35) {
        matColor = vec3(0.12, 0.12, 0.14); // Dark rubber
        spec *= 0.2;
    }
    // Windshield/glass detection (higher Y, steep normal)
    else if (vModelPos.y > 0.15 && vModelPos.y < 0.55 && abs(vModelNormal.z) > 0.35 && vModelNormal.y > 0.2) {
        matColor = vec3(0.20, 0.25, 0.32); // Tinted glass
        spec *= 1.5;
    }
    
    vec3 ambient = vec3(0.32, 0.33, 0.38);
    vec3 lightCol = vec3(1.0, 0.98, 0.95);
    vec3 litColor = matColor * (ambient + (diff1 + diff2) * lightCol) + vec3(spec);
    
    // Subtle rim lighting for crisp edges
    float rim = 1.0 - max(dot(norm, viewDir), 0.0);
    rim = pow(rim, 3.0) * 0.3;
    litColor += rim * vec3(0.7, 0.8, 1.0);
    
    FragColor = vec4(litColor, 1.0);
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

    for (int i = 0; i < FBO_COUNT; ++i) {
        if (fbos[i]) glDeleteFramebuffers(1, &fbos[i]);
        if (fbo_textures[i]) glDeleteTextures(1, &fbo_textures[i]);
        if (fbo_depths[i]) glDeleteRenderbuffers(1, &fbo_depths[i]);
    }
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

    for (int i = 0; i < FBO_COUNT; ++i) {
        glGenFramebuffers(1, &fbos[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
        
        glGenTextures(1, &fbo_textures[i]);
        glBindTexture(GL_TEXTURE_2D, fbo_textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FBO_SIZE, FBO_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_textures[i], 0);
        
        glGenRenderbuffers(1, &fbo_depths[i]);
        glBindRenderbuffer(GL_RENDERBUFFER, fbo_depths[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, FBO_SIZE, FBO_SIZE);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_depths[i]);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    LOGI("DffRenderer initialized with FBO pool and shaders.");
}

void DffRenderer::begin_frame() {
    current_fbo_index = 0;
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
    } else if (model_id >= 321 && model_id <= 372) {
        switch (model_id) {
            case 331: dff_name = "brassknuckle.dff"; break;
            case 334: dff_name = "nitestick.dff"; break;
            case 335: dff_name = "knifecur.dff"; break;
            case 336: dff_name = "bat.dff"; break;
            case 337: dff_name = "shovel.dff"; break;
            case 339: dff_name = "katana.dff"; break;
            case 341: dff_name = "chnsaw.dff"; break;
            case 342: dff_name = "grenade.dff"; break;
            case 344: dff_name = "molotov.dff"; break;
            case 346: dff_name = "colt45.dff"; break;
            case 347: dff_name = "silenced.dff"; break;
            case 348: dff_name = "desert_eagle.dff"; break;
            case 349: dff_name = "chromegun.dff"; break;
            case 350: dff_name = "sawnoff.dff"; break;
            case 351: dff_name = "shotgspa.dff"; break;
            case 352: dff_name = "micro_uzi.dff"; break;
            case 353: dff_name = "mp5lng.dff"; break;
            case 355: dff_name = "ak47.dff"; break;
            case 356: dff_name = "m4.dff"; break;
            case 357: dff_name = "cuntgun.dff"; break;
            case 358: dff_name = "sniper.dff"; break;
            case 359: dff_name = "rocketla.dff"; break;
            case 360: dff_name = "heatseek.dff"; break;
            case 361: dff_name = "flame.dff"; break;
            case 362: dff_name = "minigun.dff"; break;
            case 371: dff_name = "gun_para.dff"; break;
            case 372: dff_name = "tec9.dff"; break;
            default:  dff_name = "m4.dff"; break;
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
    
    // If specific DFF is not found, map to closest authentic GTA SA model (NEVER dummy cube!)
    if (!loaded) {
        std::string fallback_dff = "infernus.dff";
        if (model_id >= 400 && model_id <= 611) {
            if (model_id == 509 || model_id == 481 || model_id == 510) {
                fallback_dff = "mtbike.dff";
            } else if (model_id >= 461 && model_id <= 468) {
                fallback_dff = "pcj600.dff";
            } else if (model_id >= 511 && model_id <= 520) {
                fallback_dff = "shamal.dff";
            } else if (model_id >= 487 && model_id <= 488) {
                fallback_dff = "maverick.dff";
            } else if (model_id >= 446 && model_id <= 454) {
                fallback_dff = "speeder.dff";
            } else {
                fallback_dff = "infernus.dff";
            }
        } else if (model_id >= 321 && model_id <= 372) {
            fallback_dff = "m4.dff";
        } else {
            fallback_dff = "fam1.dff";
        }
        
        auto bytes = AssetManager::get().load_model_dff(fallback_dff);
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

static void parse_rw_geometries_recursive(const uint8_t* data, size_t offset, size_t end,
                                         std::vector<DffVertex>& out_verts,
                                         std::vector<uint16_t>& out_indices,
                                         float out_bounds[4]) {
    while (offset + 12 <= end) {
        uint32_t ctype = *reinterpret_cast<const uint32_t*>(data + offset);
        uint32_t csize = *reinterpret_cast<const uint32_t*>(data + offset + 4);
        size_t header_end = offset + 12;
        size_t chunk_end = std::min(header_end + csize, end);
        
        if (ctype == 0x0F) { // rwID_GEOMETRY
            if (header_end + 12 <= chunk_end) {
                uint32_t stype = *reinterpret_cast<const uint32_t*>(data + header_end);
                if (stype == 0x01) {
                    const uint8_t* ptr = data + header_end + 12;
                    uint16_t flags = *reinterpret_cast<const uint16_t*>(ptr);
                    uint16_t num_uv = *reinterpret_cast<const uint16_t*>(ptr + 2);
                    uint32_t num_tris = *reinterpret_cast<const uint32_t*>(ptr + 4);
                    uint32_t num_verts = *reinterpret_cast<const uint32_t*>(ptr + 8);
                    
                    if (num_verts > 0 && num_tris > 0 && (out_verts.size() + num_verts) < 65535) {
                        size_t cur = header_end + 12 + 16;
                        if (flags & 0x0008) cur += num_verts * 4; // prelit colors
                        const float* uvs = nullptr;
                        uint16_t actual_uv = (num_uv > 0) ? num_uv : ((flags & 0x0004) ? 1 : 0);
                        if (actual_uv > 0) {
                            uvs = reinterpret_cast<const float*>(data + cur);
                            cur += num_verts * 8 * actual_uv;
                        }
                        
                        const uint16_t* tris = reinterpret_cast<const uint16_t*>(data + cur);
                        cur += num_tris * 8;
                        
                        if (cur + 24 <= chunk_end) {
                            const float* bounds = reinterpret_cast<const float*>(data + cur);
                            if (bounds[3] > out_bounds[3]) {
                                // GTA coords -> OpenGL: X=Right, Y=Up (GTA Z), Z=-Forward (GTA -Y)
                                out_bounds[0] = bounds[0];
                                out_bounds[1] = bounds[2];
                                out_bounds[2] = -bounds[1];
                                out_bounds[3] = bounds[3];
                            }
                            cur += 16;
                            uint32_t has_verts = *reinterpret_cast<const uint32_t*>(data + cur);
                            uint32_t has_normals = *reinterpret_cast<const uint32_t*>(data + cur + 4);
                            cur += 8;
                            
                            if (has_verts && cur + num_verts * 12 <= chunk_end) {
                                const float* vert_pos = reinterpret_cast<const float*>(data + cur);
                                cur += num_verts * 12;
                                const float* vert_norm = nullptr;
                                if (has_normals && cur + num_verts * 12 <= chunk_end) {
                                    vert_norm = reinterpret_cast<const float*>(data + cur);
                                }
                                
                                uint16_t base_index = (uint16_t)out_verts.size();
                                for (uint32_t i = 0; i < num_verts; ++i) {
                                    DffVertex v;
                                    // GTA coords: X=Right, Y=Forward, Z=Up
                                    // OpenGL coords: X=Right, Y=Up (GTA Z), Z=-Forward (GTA -Y)
                                    v.x = vert_pos[i * 3 + 0];
                                    v.y = vert_pos[i * 3 + 2];
                                    v.z = -vert_pos[i * 3 + 1];
                                    if (vert_norm) {
                                        v.nx = vert_norm[i * 3 + 0];
                                        v.ny = vert_norm[i * 3 + 2];
                                        v.nz = -vert_norm[i * 3 + 1];
                                    } else {
                                        v.nx = 0.0f; v.ny = 1.0f; v.nz = 0.0f;
                                    }
                                    if (uvs) {
                                        v.u = uvs[i * 2 + 0];
                                        v.v = uvs[i * 2 + 1];
                                    } else {
                                        v.u = 0.0f; v.v = 0.0f;
                                    }
                                    out_verts.push_back(v);
                                }
                                
                                for (uint32_t i = 0; i < num_tris; ++i) {
                                    uint16_t v2 = tris[i * 4 + 0];
                                    uint16_t v1 = tris[i * 4 + 1];
                                    uint16_t v3 = tris[i * 4 + 3];
                                    if (v1 < num_verts && v2 < num_verts && v3 < num_verts) {
                                        out_indices.push_back(base_index + v1);
                                        out_indices.push_back(base_index + v2);
                                        out_indices.push_back(base_index + v3);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else if (ctype == 0x10 || ctype == 0x1A) { // rwID_CLUMP or rwID_GEOMETRYLIST
            parse_rw_geometries_recursive(data, header_end, chunk_end, out_verts, out_indices, out_bounds);
        }
        offset = chunk_end;
    }
}

bool DffRenderer::parse_dff_data(const std::vector<uint8_t>& data, DffMesh& out_mesh) {
    if (data.size() < 64) return false;
    
    std::vector<DffVertex> verts;
    std::vector<uint16_t> indices;
    float bounds[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    
    parse_rw_geometries_recursive(data.data(), 0, data.size(), verts, indices, bounds);
    
    if (verts.empty() || indices.empty()) {
        return false;
    }
    
    if (bounds[3] <= 0.1f) {
        float min_x = verts[0].x, max_x = verts[0].x;
        float min_y = verts[0].y, max_y = verts[0].y;
        float min_z = verts[0].z, max_z = verts[0].z;
        for (const auto& v : verts) {
            min_x = std::min(min_x, v.x); max_x = std::max(max_x, v.x);
            min_y = std::min(min_y, v.y); max_y = std::max(max_y, v.y);
            min_z = std::min(min_z, v.z); max_z = std::max(max_z, v.z);
        }
        bounds[0] = (min_x + max_x) * 0.5f;
        bounds[1] = (min_y + max_y) * 0.5f;
        bounds[2] = (min_z + max_z) * 0.5f;
        float dx = max_x - min_x, dy = max_y - min_y, dz = max_z - min_z;
        bounds[3] = std::sqrt(dx*dx + dy*dy + dz*dz) * 0.5f;
    }
    
    out_mesh.bound_sphere[0] = bounds[0];
    out_mesh.bound_sphere[1] = bounds[1];
    out_mesh.bound_sphere[2] = bounds[2];
    out_mesh.bound_sphere[3] = (bounds[3] > 0.1f) ? bounds[3] : 2.5f;
    
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
    
    LOGI("Successfully loaded DFF mesh with %zu vertices, %zu indices (radius=%.2f)",
         verts.size(), indices.size(), out_mesh.bound_sphere[3]);
    return true;
}

void DffRenderer::create_fallback_mesh(DffMesh& out_mesh) {
    // Stylized low-poly car mesh fallback
    std::vector<DffVertex> verts = {
        // Hood & Front
        {-0.9f, -0.4f,  1.7f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f},
        { 0.9f, -0.4f,  1.7f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f},
        { 0.85f, 0.3f,  0.8f,  0.0f,  0.7f,  0.7f, 1.0f, 0.5f},
        {-0.85f, 0.3f,  0.8f,  0.0f,  0.7f,  0.7f, 0.0f, 0.5f},
        // Cabin Roof
        {-0.75f, 0.7f, -0.2f,  0.0f,  1.0f,  0.0f, 0.0f, 0.8f},
        { 0.75f, 0.7f, -0.2f,  0.0f,  1.0f,  0.0f, 1.0f, 0.8f},
        { 0.75f, 0.7f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f},
        {-0.75f, 0.7f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f},
        // Trunk & Rear
        { 0.85f, 0.35f,-1.7f,  0.0f,  0.2f, -0.9f, 1.0f, 0.5f},
        {-0.85f, 0.35f,-1.7f,  0.0f,  0.2f, -0.9f, 0.0f, 0.5f},
        {-0.9f, -0.4f, -1.8f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f},
        { 0.9f, -0.4f, -1.8f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f},
    };
    std::vector<uint16_t> indices = {
        0, 1, 2,  0, 2, 3,       // Front Hood
        3, 2, 5,  3, 5, 4,       // Windshield
        4, 5, 6,  4, 6, 7,       // Roof
        7, 6, 8,  7, 8, 9,       // Rear Window
        9, 8, 11, 9, 11, 10,     // Trunk
        0, 3, 4,  0, 4, 10,      // Left Side
        1, 11, 5, 1, 5, 2,       // Right Side
        0, 10, 11, 0, 11, 1      // Bottom
    };
    
    out_mesh.bound_sphere[0] = 0.0f;
    out_mesh.bound_sphere[1] = 0.1f;
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

GLuint DffRenderer::render_to_texture(int model_id, float rot_x, float rot_y, float rot_z, float zoom,
                                      int veh_col1, int veh_col2) {
    if (!shader_program || fbos[0] == 0) return 0;
    
    DffMesh mesh;
    load_model_mesh(model_id, mesh);
    if (!mesh.vao || mesh.index_count == 0) return 0;
    
    int fbo_idx = current_fbo_index % FBO_COUNT;
    current_fbo_index++;
    
    GLint orig_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &orig_fbo);
    GLint orig_vp[4];
    glGetIntegerv(GL_VIEWPORT, orig_vp);
    GLboolean orig_scissor = glIsEnabled(GL_SCISSOR_TEST);
    glDisable(GL_SCISSOR_TEST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[fbo_idx]);
    glViewport(0, 0, FBO_SIZE, FBO_SIZE);
    
    // Clear transparent background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(shader_program);
    
    float aspect = 1.0f;
    Mat4 proj = Mat4::perspective(45.0f * (3.14159265f / 180.0f), aspect, 0.1f, 100.0f);
    
    float radius = mesh.bound_sphere[3];
    if (radius <= 0.1f) radius = 2.0f;
    float dist = (radius * 2.6f) / std::max(zoom, 0.1f);
    Mat4 view = Mat4::translation(0.0f, 0.0f, -dist);
    
    const float to_rad = 3.14159265f / 180.0f;
    Mat4 rot = Mat4::rotation_zyx(rot_x * to_rad, rot_y * to_rad, rot_z * to_rad);
    Mat4 center = Mat4::translation(-mesh.bound_sphere[0], -mesh.bound_sphere[1], -mesh.bound_sphere[2]);
    
    Mat4 model = Mat4::multiply(rot, center);
    Mat4 mv = Mat4::multiply(view, model);
    Mat4 mvp = Mat4::multiply(proj, mv);
    
    glUniformMatrix4fv(u_mvp_loc, 1, GL_FALSE, mvp.m);
    glUniformMatrix4fv(u_model_loc, 1, GL_FALSE, model.m);
    
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
    
    glDisable(GL_DEPTH_TEST);
    
    // Restore states
    glBindFramebuffer(GL_FRAMEBUFFER, orig_fbo);
    glViewport(orig_vp[0], orig_vp[1], orig_vp[2], orig_vp[3]);
    if (orig_scissor) glEnable(GL_SCISSOR_TEST);
    
    return fbo_textures[fbo_idx];
}
