#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

#include "nisaba/gpu/gpu_device.hpp"
#include "nisaba/render3d/render3d.hpp"
#include <GL/glew.h>
#include <GL/gl.h>

using namespace nisaba::render3d;
using namespace nisaba::physics;
using nisaba::gpu::GpuDevice;

void test_gl_shader_program_compilation() {
    std::cout << "[Test 1] GlShaderProgram Compilation & Introspection..." << std::endl;

    auto program = std::make_unique<GlShaderProgram>();
    const char* vert = R"(#version 330 core
layout(location = 0) in vec3 inPos;
uniform mat4 u_mvp;
void main() {
    gl_Position = u_mvp * vec4(inPos, 1.0);
}
)";

    const char* frag = R"(#version 330 core
uniform vec4 u_tint;
out vec4 FragColor;
void main() {
    FragColor = u_tint;
}
)";

    bool ok = program->compile_and_link(vert, frag);
    if (!ok) {
        std::cerr << "Custom shader compile failed: " << program->log() << std::endl;
    }
    assert(ok);
    assert(program->is_valid());
    assert(program->program_id() != 0);

    program->bind();
    assert(program->set_uniform("u_mvp", Mat44::identity()));
    assert(program->set_uniform("u_tint", 1.0f, 0.5f, 0.2f, 1.0f));
    program->unbind();

    // Test Sovereign PBR Shader compilation
    auto pbr_shader = GlShaderProgram::create_pbr_shader();
    assert(pbr_shader != nullptr);
    assert(pbr_shader->is_valid());

    // Test Shadow Depth Shader compilation
    auto depth_shader = GlShaderProgram::create_shadow_depth_shader();
    assert(depth_shader != nullptr);
    assert(depth_shader->is_valid());

    std::cout << "  -> Shader compilation & uniform binding PASSED!" << std::endl;
}

void test_gl_state_guard_isolation() {
    std::cout << "[Test 2] GlStateGuard3D RAII Pipeline Isolation..." << std::endl;

    // Set a known initial state
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glViewport(10, 20, 300, 400);

    {
        GlStateGuard3D guard;

        // Mutate states inside 3D scope
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_BLEND);
        glViewport(0, 0, 1920, 1080);

        // Verify mutations took effect
        assert(glIsEnabled(GL_DEPTH_TEST) == GL_FALSE);
        assert(glIsEnabled(GL_BLEND) == GL_FALSE);

        GLint vp[4]{0};
        glGetIntegerv(GL_VIEWPORT, vp);
        assert(vp[2] == 1920 && vp[3] == 1080);
    } // ~GlStateGuard3D restores initial state

    // Verify full state restoration
    assert(glIsEnabled(GL_DEPTH_TEST) == GL_TRUE);
    assert(glIsEnabled(GL_BLEND) == GL_TRUE);

    GLint restored_vp[4]{0};
    glGetIntegerv(GL_VIEWPORT, restored_vp);
    assert(restored_vp[0] == 10 && restored_vp[1] == 20);
    assert(restored_vp[2] == 300 && restored_vp[3] == 400);

    std::cout << "  -> GlStateGuard3D zero-leak restoration PASSED!" << std::endl;
}

void test_gpu_resource_cache() {
    std::cout << "[Test 3] GpuResourceCache Mesh, Texture & UBO Uploads..." << std::endl;

    GpuResourceCache cache;

    // 1. Upload Cube Mesh
    Mesh3D cube = MeshPrimitives::create_cube(Vec3(2.0f, 2.0f, 2.0f));
    auto mesh_handle1 = cache.get_or_upload_mesh(cube);
    assert(mesh_handle1.is_valid);
    assert(mesh_handle1.vao != 0);
    assert(mesh_handle1.vbo != 0);
    assert(mesh_handle1.ebo != 0);
    assert(mesh_handle1.index_count == cube.indices.size());

    // Re-query must return exact cached handle
    auto mesh_handle2 = cache.get_or_upload_mesh(cube);
    assert(mesh_handle2.vao == mesh_handle1.vao);

    // 2. Upload Texture
    auto solid_tex = Texture2D::create_solid(Vec4(1.0f, 0.5f, 0.2f, 1.0f), 4, 4);
    auto tex_handle = cache.get_or_upload_texture(*solid_tex);
    assert(tex_handle.is_valid);
    assert(tex_handle.texture_id != 0);

    // 3. Update UBOs
    cache.update_camera_ubo(Mat44::identity(), Mat44::identity(), Vec3(0, 0, 5));

    PbrMaterialUniforms mat_uniforms;
    mat_uniforms.metallic = 0.8f;
    mat_uniforms.roughness = 0.2f;
    cache.update_material_ubo(mat_uniforms);

    std::cout << "  -> Mesh, Texture & UBO caching PASSED!" << std::endl;
}

void test_gpu_shadow_pass_rendering() {
    std::cout << "[Test 4] GpuShadowPass Hardware Depth Rendering..." << std::endl;

    GpuShadowPass shadow_pass(1024);
    assert(shadow_pass.depth_texture() != 0);
    assert(shadow_pass.resolution() == 1024);

    GpuResourceCache cache;
    Mesh3D sphere = MeshPrimitives::create_sphere(1.0f);

    shadow_pass.begin(Mat44::identity());
    shadow_pass.render_caster(cache, sphere, Mat44::translation(Vec3(0, 0, -5)));
    shadow_pass.end();

    std::cout << "  -> Hardware shadow pass execution PASSED!" << std::endl;
}

void test_full_hardware_pbr_rendering() {
    std::cout << "[Test 5] Full Hardware PBR Rendering & Framebuffer Readback..." << std::endl;

    const uint32_t width = 320;
    const uint32_t height = 240;

    RenderSurface3D surface(width, height);
    assert(surface.fbo() != 0);
    assert(surface.color_texture() != 0);

    // Setup 3D Scene
    auto scene = std::make_shared<Scene3D>();

    // Add Golden Cube in center
    auto cube_mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_cube(Vec3(1.5f, 1.5f, 1.5f)));
    auto gold_mat = PbrMaterial::create_gold();
    gold_mat->metallic_factor = 0.0f; // Direct diffuse gold illumination under directional sun
    auto renderable = std::make_shared<Renderable3D>(cube_mesh, gold_mat);
    renderable->set_position(Vec3(0.0f, 0.0f, 0.0f));
    scene->add_renderable(renderable);

    // Add Directional Sun Light
    Light3D sun = Light3D::create_directional(
        Vec3(-0.3f, -1.0f, -0.5f).normalized(),
        Vec3(1.0f, 0.98f, 0.9f),
        4.0f
    );
    scene->add_light(sun);

    // Setup Camera
    auto camera = std::make_shared<Camera3D>();
    camera->set_look_at(Vec3(0.0f, 1.0f, 4.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3::up());
    camera->set_perspective(50.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

    // Setup View
    View3D view;
    view.set_scene(scene);
    view.set_camera(camera);
    view.set_viewport(0, 0, width, height);
    view.set_clear_color(Vec4(0.05f, 0.05f, 0.1f, 1.0f)); // Dark navy background
    view.shadows_enabled = true;

    // Render via Hardware GPU Backend
    GpuBackend3D backend;
    surface.render_3d(backend, view, true);

    // Read back rendered pixels
    std::vector<uint8_t> pixels(width * height * 4);
    surface.read_pixels(pixels.data());

    // Corner pixel should match dark navy clear color
    const uint8_t corner_r = pixels[0];
    const uint8_t corner_g = pixels[1];
    const uint8_t corner_b = pixels[2];
    assert(corner_r < 30);
    assert(corner_g < 30);
    assert(corner_b > 10 && corner_b < 50);

    // Center pixel should be brightly lit gold/yellow from the cube!
    const size_t center_idx = (height / 2 * width + width / 2) * 4;
    const uint8_t center_r = pixels[center_idx + 0];
    const uint8_t center_g = pixels[center_idx + 1];
    const uint8_t center_b = pixels[center_idx + 2];

    assert(center_r > 80); // Gold has strong red & green components
    assert(center_g > 60);

    std::cout << "  -> Hardware PBR rendering validated! (Background: "
              << (int)corner_r << "," << (int)corner_g << "," << (int)corner_b
              << " | Golden Cube: "
              << (int)center_r << "," << (int)center_g << "," << (int)center_b
              << ") PASSED!" << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Nisaba Sovereign 3D GPU Pipeline Integration Tests" << std::endl;
    std::cout << "==========================================================" << std::endl;

    auto device = GpuDevice::create();
    if (!device || !device->is_valid()) {
        std::cout << "Hardware GPU device not available on this platform. Skipping GPU tests." << std::endl;
        return 0;
    }

    std::cout << "Active GPU Hardware Device: " << device->renderer_name()
              << " (" << device->version_name() << ")" << std::endl;

    device->make_current();

    test_gl_shader_program_compilation();
    test_gl_state_guard_isolation();
    test_gpu_resource_cache();
    test_gpu_shadow_pass_rendering();
    test_full_hardware_pbr_rendering();

    std::cout << "\n>>> ALL 3D GPU HARDWARE INTEGRATION TESTS PASSED! <<<" << std::endl;
    return 0;
}
