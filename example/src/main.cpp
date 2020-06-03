/* clang-format off */
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
/* clang-format on */

#include <aryibi/renderer.hpp>
#include <aryibi/sprites.hpp>
#include <aryibi/sprite_solvers.hpp>
#include <iostream>
#include <map>

static void debug_callback(GLenum const source,
                           GLenum const type,
                           GLuint,
                           GLenum const severity,
                           GLsizei,
                           GLchar const* const message,
                           void const*) {
    auto stringify_source = [](GLenum const source) {
        switch (source) {
            case GL_DEBUG_SOURCE_API: return u8"API";
            case GL_DEBUG_SOURCE_APPLICATION: return u8"Application";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return u8"Shader Compiler";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return u8"Window System";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return u8"Third Party";
            case GL_DEBUG_SOURCE_OTHER: return u8"Other";
            default: return "";
        }
    };

    auto stringify_type = [](GLenum const type) {
        switch (type) {
            case GL_DEBUG_TYPE_ERROR: return u8"Error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return u8"Deprecated Behavior";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return u8"Undefined Behavior";
            case GL_DEBUG_TYPE_PORTABILITY: return u8"Portability";
            case GL_DEBUG_TYPE_PERFORMANCE: return u8"Performance";
            case GL_DEBUG_TYPE_MARKER: return u8"Marker";
            case GL_DEBUG_TYPE_PUSH_GROUP: return u8"Push Group";
            case GL_DEBUG_TYPE_POP_GROUP: return u8"Pop Group";
            case GL_DEBUG_TYPE_OTHER: return u8"Other";
            default: return "";
        }
    };

    auto stringify_severity = [](GLenum const severity) {
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH: return u8"Fatal Error";
            case GL_DEBUG_SEVERITY_MEDIUM: return u8"Error";
            case GL_DEBUG_SEVERITY_LOW: return u8"Warning";
            case GL_DEBUG_SEVERITY_NOTIFICATION: return u8"Note";
            default: return "";
        }
    };

    // Do not send bloat information
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    std::cout << "[" << stringify_severity(severity) << ":" << stringify_type(type) << " in "
              << stringify_source(source) << "]: " << message << std::endl;
}

namespace rnd = aryibi::renderer;
namespace spr = aryibi::sprites;

namespace {

std::unique_ptr<aryibi::renderer::Renderer> renderer;
GLFWwindow* window = nullptr;

bool init() {
    if (!glfwInit()) {
        std::cerr << "Couldn't init GLFW." << std::endl;
        return false;
    }

    // Use OpenGL 4.5
    const char* glsl_version = "#version 450";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    window = glfwCreateWindow(400, 400, "Aryibi example", nullptr, nullptr);
    if (!window) {
        std::cerr
            << "Couldn't create window. Check your GPU drivers, as aryibi requires OpenGL 4.5."
            << std::endl;
        return false;
    }

    // Activate VSync and fix FPS
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // TODO: Change to 1 to enable VSync
    gladLoadGLLoader((GLADloadproc)&glfwGetProcAddress);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glCullFace(GL_FRONT_AND_BACK);

    glDebugMessageCallback(debug_callback, nullptr);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    renderer = std::make_unique<aryibi::renderer::Renderer>(window);

    return true;
}

struct CommonDemoData {
    rnd::TextureHandle tiles_tex, directional_8_tex, directional_4_tex, colors_tex;
    spr::TextureChunk rpgmaker_a2_example_chunk, directional_8_example_chunk,
        directional_4_example_chunk, red_chunk, green_chunk;
    rnd::MeshHandle rpgmaker_a2_full_mesh, directional_8_full_mesh, directional_4_full_mesh;
    rnd::MeshBuilder builder;
};

void sprite_types_demo(CommonDemoData& c) {
    static auto directional_8_direction = spr::direction::dir_down;
    static spr::Tile8Connections rpgmaker_a2_example_connections;
    const double time = glfwGetTime();
    const float normalized_sin = (aml::sin(time) + 1.f) / 2.f;
    const float normalized_cos = (aml::cos(time) + 1.f) / 2.f;
    const auto clear_color = rnd::Color(normalized_sin, normalized_cos,
                                        aml::max(0.f, 1.f - normalized_sin - normalized_cos), 1);
    renderer->start_frame(clear_color);
    rnd::DrawCmdList cmd_list;
    cmd_list.camera = {{0, 0, 0}, 32};
    rnd::DrawCmd rpgmaker_a2_full_mesh_draw_command{
        c.tiles_tex, c.rpgmaker_a2_full_mesh, renderer->lit_shader(), {{-3, -1.5, 0}}};
    cmd_list.commands.emplace_back(rpgmaker_a2_full_mesh_draw_command);
    rnd::DrawCmd directional_8_full_mesh_draw_command{
        c.directional_8_tex, c.directional_8_full_mesh, renderer->unlit_shader(), {{-8, 3.5, 0}}};
    cmd_list.commands.emplace_back(directional_8_full_mesh_draw_command);
    rnd::DrawCmd directional_4_full_mesh_draw_command{
        c.directional_4_tex, c.directional_4_full_mesh, renderer->unlit_shader(), {{-4, 1.5, 0}}};
    cmd_list.commands.emplace_back(directional_4_full_mesh_draw_command);

    c.builder.add_sprite(
        spr::solve_rpgmaker_a2(c.rpgmaker_a2_example_chunk, rpgmaker_a2_example_connections),
        {0, 0, 0});
    auto rpgmaker_a2_tile_mesh = c.builder.finish();
    rnd::DrawCmd rpgmaker_a2_tile_mesh_draw_command{c.tiles_tex,
                                                    rpgmaker_a2_tile_mesh,
                                                    renderer->unlit_shader(),
                                                    {{-normalized_sin * 2.f, -.5, .5f}},
                                                    true};
    cmd_list.commands.emplace_back(rpgmaker_a2_tile_mesh_draw_command);

    c.builder.add_sprite(
        spr::solve_8_directional(c.directional_8_example_chunk, directional_8_direction, {5, 5}),
        {0, 0, 0});
    auto directional_8_sprite_mesh = c.builder.finish();
    rnd::DrawCmd directional_8_tile_mesh_draw_command{
        c.directional_8_tex, directional_8_sprite_mesh, renderer->unlit_shader(), {{0, -7.f, 0}}};
    cmd_list.commands.emplace_back(directional_8_tile_mesh_draw_command);

    c.builder.add_sprite(
        spr::solve_4_directional(c.directional_4_example_chunk, directional_8_direction, {5, 5}),
        {0, 0, 0});
    auto directional_4_sprite_mesh = c.builder.finish();
    rnd::DrawCmd directional_4_tile_mesh_draw_command{c.directional_4_tex,
                                                      directional_4_sprite_mesh,
                                                      renderer->unlit_shader(),
                                                      {{-5.f, -7.f, 0}}};
    cmd_list.commands.emplace_back(directional_4_tile_mesh_draw_command);

    renderer->draw(cmd_list, renderer->get_window_framebuffer());
    rpgmaker_a2_tile_mesh.unload();
    directional_8_sprite_mesh.unload();

    renderer->finish_frame();

    rpgmaker_a2_example_connections.down_left = glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_RELEASE;
    rpgmaker_a2_example_connections.down = glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_RELEASE;
    rpgmaker_a2_example_connections.down_right = glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_RELEASE;
    rpgmaker_a2_example_connections.left = glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_RELEASE;
    rpgmaker_a2_example_connections.right = glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_RELEASE;
    rpgmaker_a2_example_connections.up_left = glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_RELEASE;
    rpgmaker_a2_example_connections.up = glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_RELEASE;
    rpgmaker_a2_example_connections.up_right = glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_RELEASE;

    std::map<spr::direction::Direction, spr::direction::Direction> directional_8_next_dir = {
        {spr::direction::dir_down_left, spr::direction::dir_down},
        {spr::direction::dir_down, spr::direction::dir_down_right},
        {spr::direction::dir_down_right, spr::direction::dir_right},
        {spr::direction::dir_right, spr::direction::dir_up_right},
        {spr::direction::dir_up_right, spr::direction::dir_up},
        {spr::direction::dir_up, spr::direction::dir_up_left},
        {spr::direction::dir_up_left, spr::direction::dir_left},
        {spr::direction::dir_left, spr::direction::dir_down_left},
    };
    static double last_direction_change_time = time;
    if (time > last_direction_change_time + 0.15) {
        directional_8_direction = directional_8_next_dir[directional_8_direction];
        last_direction_change_time = time;
    }
}

void lighting_demo(CommonDemoData& c) {
    const auto create_ground_mesh = [&c]() -> rnd::MeshHandle {
        c.builder.add_sprite(spr::solve_normal(c.red_chunk, {20, 20}), {0, 0, 0});
        return c.builder.finish();
    };
    const auto create_quad_mesh = [&c]() -> rnd::MeshHandle {
        c.builder.add_sprite(spr::solve_normal(c.green_chunk, {1, 1}), {0, 0, 0});
        return c.builder.finish();
    };
    static const auto ground_mesh = create_ground_mesh();
    static const auto quad_mesh = create_quad_mesh();

    const double time = glfwGetTime();
    const float normalized_sin = (aml::sin(time) + 1.f) / 2.f;
    const float normalized_cos = (aml::cos(time) + 1.f) / 2.f;

    renderer->start_frame(rnd::colors::white);
    rnd::DrawCmdList cmd_list;
    cmd_list.camera = {{0, 0, 0}, 32};
    rnd::DrawCmd ground_draw_command{
        c.colors_tex, ground_mesh, renderer->lit_shader(), {{-10, -10, 0}}};
    cmd_list.commands.emplace_back(ground_draw_command);

    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 10; ++y) {
            float z = (aml::sin(time + x + y) + 1.f) / 2.f;
            float w = (aml::cos(time + x + y) + 1.f) / 2.f;
            rnd::DrawCmd ground_draw_command{c.colors_tex,
                                             quad_mesh,
                                             renderer->lit_shader(),
                                             {{-5.f + (float)x * 1.2f + w, 5.f - (float)y * 1.2f + z,
                                               z}},
                                             true};
            cmd_list.commands.emplace_back(ground_draw_command);
        }
    }

    renderer->draw(cmd_list, renderer->get_window_framebuffer());
    renderer->finish_frame();
}

} // namespace

int main() {
    if (!init())
        return -1;

    CommonDemoData common_data;

    common_data.tiles_tex = rnd::TextureHandle::from_file_rgba("assets/tiles_packed.png");
    assert(common_data.tiles_tex.exists());
    common_data.directional_8_tex =
        rnd::TextureHandle::from_file_rgba("assets/pato_dando_vueltas.png");
    assert(common_data.directional_8_tex.exists());
    common_data.directional_4_tex =
        rnd::TextureHandle::from_file_rgba("assets/pato_dando_vueltas_4.png");
    assert(common_data.directional_4_tex.exists());
    common_data.colors_tex = rnd::TextureHandle::from_file_rgba("assets/colors.png");
    assert(common_data.colors_tex.exists());
    common_data.rpgmaker_a2_example_chunk =
        spr::TextureChunk{common_data.tiles_tex, {{0, 0}, {1.f / 4.f, 1.f / 2.f}}};
    common_data.directional_8_example_chunk =
        spr::TextureChunk::full(common_data.directional_8_tex);
    common_data.directional_4_example_chunk =
        spr::TextureChunk::full(common_data.directional_4_tex);
    common_data.red_chunk =
        spr::TextureChunk{common_data.colors_tex, {{3.f / 5.f, 0}, {4.f / 5.f, 1}}};
    common_data.green_chunk =
        spr::TextureChunk{common_data.colors_tex, {{1.f / 5.f, 0}, {2.f / 5.f, 1}}};

    common_data.builder.add_sprite(spr::solve_normal(common_data.rpgmaker_a2_example_chunk, {2, 3}),
                                   {0, 0, 0});
    common_data.rpgmaker_a2_full_mesh = common_data.builder.finish();
    common_data.builder.add_sprite(
        spr::solve_normal(common_data.directional_8_example_chunk, {16, 2}), {0, 0, 0});
    common_data.directional_8_full_mesh = common_data.builder.finish();
    common_data.builder.add_sprite(
        spr::solve_normal(common_data.directional_4_example_chunk, {8, 2}), {0, 0, 0});
    common_data.directional_4_full_mesh = common_data.builder.finish();

    enum class Demo : anton::u8 { sprite_types, lighting, count } demo = Demo::sprite_types;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        switch (demo) {
            case Demo::sprite_types: sprite_types_demo(common_data); break;
            case Demo::lighting: lighting_demo(common_data); break;
            default: break;
        }

        static bool pressed_space_before = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool pressed_space_now = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (!pressed_space_before && pressed_space_now) {
            demo = (Demo)(((anton::u8)demo) + 1u);
            if (demo == Demo::count)
                demo = (Demo)0;
        }
        pressed_space_before = pressed_space_now;
    }
}