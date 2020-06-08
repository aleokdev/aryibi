#include <aryibi/renderer.hpp>
#include <aryibi/sprites.hpp>
#include <aryibi/windowing.hpp>
#include <aryibi/sprite_solvers.hpp>

#include <imgui.h>

#include <iostream>
#include <map>

namespace aml = anton::math;

namespace rnd = aryibi::renderer;
namespace wnd = aryibi::windowing;
namespace spr = aryibi::sprites;

namespace {

std::unique_ptr<aryibi::renderer::Renderer> renderer;
wnd::WindowHandle window;
wnd::InputHandle input;

bool init() {
    window.init(800, 800, "Aryibi example", {{{wnd::WindowHint::transparent_background, true}}});
    if (!window.exists()) {
        std::cerr << "Couldn't init the window." << std::endl;
        return false;
    }

    input.init(window);
    if (!input.exists()) {
        std::cerr << "Couldn't init the input." << std::endl;
        return false;
    }

    renderer = std::make_unique<aryibi::renderer::Renderer>(window);
    renderer->set_shadow_resolution(2048, 2048);

    return true;
}

struct CommonDemoData {
    rnd::TextureHandle tiles_tex, directional_8_tex, directional_4_tex, colors_tex;
    spr::TextureChunk rpgmaker_a2_example_chunk, directional_8_example_chunk,
        directional_4_example_chunk, red_chunk, green_chunk;
    rnd::MeshHandle rpgmaker_a2_full_mesh, rpgmaker_a2_all_tiles_mesh, directional_8_full_mesh,
        directional_4_full_mesh;
    rnd::MeshBuilder builder;
};

void sprite_types_demo(CommonDemoData& c) {
    static auto directional_8_direction = spr::direction::dir_down;
    const double time = window.time_since_opened();
    const float normalized_sin = (aml::sin(time) + 1.f) / 2.f;
    const float normalized_cos = (aml::cos(time) + 1.f) / 2.f;
    const auto clear_color = rnd::Color(normalized_sin, normalized_cos,
                                        aml::max(0.f, 1.f - normalized_sin - normalized_cos), 1);
    renderer->start_frame(clear_color);
    ImGui::ShowMetricsWindow();

    rnd::DrawCmdList cmd_list;
    cmd_list.camera = {{0, 0, 10}, 32};
    rnd::DrawCmd rpgmaker_a2_full_mesh_draw_command{
        c.tiles_tex, c.rpgmaker_a2_full_mesh, renderer->unlit_shader(), {{-3, -1.5, 0}}};
    cmd_list.commands.emplace_back(rpgmaker_a2_full_mesh_draw_command);
    rnd::DrawCmd directional_8_full_mesh_draw_command{
        c.directional_8_tex, c.directional_8_full_mesh, renderer->unlit_shader(), {{-23, 3.5, 0}}};
    cmd_list.commands.emplace_back(directional_8_full_mesh_draw_command);
    rnd::DrawCmd directional_4_full_mesh_draw_command{
        c.directional_4_tex, c.directional_4_full_mesh, renderer->unlit_shader(), {{-19, 1.5, 0}}};
    cmd_list.commands.emplace_back(directional_4_full_mesh_draw_command);

    rnd::DrawCmd rpgmaker_a2_tile_mesh_draw_command{
        c.tiles_tex, c.rpgmaker_a2_all_tiles_mesh, renderer->unlit_shader(), {{0, -8, .5f}}};
    cmd_list.commands.emplace_back(rpgmaker_a2_tile_mesh_draw_command);

    c.builder.add_sprite(
        spr::solve_8_directional(c.directional_8_example_chunk, directional_8_direction, {5, 5}),
        {0, 0, 0});
    auto directional_8_sprite_mesh = c.builder.finish();
    rnd::DrawCmd directional_8_tile_mesh_draw_command{c.directional_8_tex,
                                                      directional_8_sprite_mesh,
                                                      renderer->unlit_shader(),
                                                      {{-15.f, -7.f, 0}}};
    cmd_list.commands.emplace_back(directional_8_tile_mesh_draw_command);

    c.builder.add_sprite(
        spr::solve_4_directional(c.directional_4_example_chunk, directional_8_direction, {5, 5}),
        {0, 0, 0});
    auto directional_4_sprite_mesh = c.builder.finish();
    rnd::DrawCmd directional_4_tile_mesh_draw_command{c.directional_4_tex,
                                                      directional_4_sprite_mesh,
                                                      renderer->unlit_shader(),
                                                      {{-20.f, -7.f, 0}}};
    cmd_list.commands.emplace_back(directional_4_tile_mesh_draw_command);

    renderer->draw(cmd_list, renderer->get_window_framebuffer());
    directional_8_sprite_mesh.unload();
    directional_4_sprite_mesh.unload();

    renderer->finish_frame();

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

    const double time = window.time_since_opened();
    const float normalized_sin = (aml::sin(time) + 1.f) / 2.f;

    renderer->start_frame(rnd::colors::transparent);
    ImGui::ShowMetricsWindow();
    rnd::DrawCmdList cmd_list;
    cmd_list.camera = {{0, 0, 10}, 32};
    rnd::PointLight point_light;
    point_light.color = rnd::colors::red;
    point_light.radius = 10.f;
    point_light.intensity = 1.f;
    point_light.position = {-10.f * normalized_sin, 0, 5.f};
    cmd_list.point_lights.emplace_back(point_light);
    point_light.color = rnd::colors::blue;
    point_light.radius = 10.f;
    point_light.intensity = 1.f;
    point_light.position = {10.f * normalized_sin, 0, 5.f};
    cmd_list.point_lights.emplace_back(point_light);
    point_light.color = rnd::colors::green;
    point_light.radius = 10.f;
    point_light.intensity = 1.f;
    point_light.position = {0, 10.f * normalized_sin, 5.f};
    cmd_list.point_lights.emplace_back(point_light);

    rnd::DrawCmd ground_draw_command{
        c.colors_tex, ground_mesh, renderer->lit_shader(), {{-10, -10, -0.5f}}, true};
    cmd_list.commands.emplace_back(ground_draw_command);


    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 10; ++y) {
            float z = (aml::sin(time + x + y) + 1.f) / 2.f;
            float w = (aml::cos(time + x + y) + 1.f) / 2.f;
            c.builder.add_sprite(spr::solve_normal(c.green_chunk, {1, 1}), {-5.f + (float)x * 1.2f + w, 5.f - (float)y * 1.2f + z, z});
        }
    }
    rnd::MeshHandle quads_mesh;
    rnd::DrawCmd quads_draw_command{
        c.colors_tex,
        quads_mesh = c.builder.finish(),
        renderer->lit_shader(),
        {{0,0,0}},
        true};
    cmd_list.commands.emplace_back(quads_draw_command);

    renderer->draw(cmd_list, renderer->get_window_framebuffer());
    quads_mesh.unload();
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

    for (unsigned int tile_i = 0; tile_i <= 0xFFu; ++tile_i) {
        const float tile_x = tile_i % 16;
        const float tile_y = tile_i / 16;
        common_data.builder.add_sprite(
            spr::solve_rpgmaker_a2(common_data.rpgmaker_a2_example_chunk,
                                   {bool(tile_i & (1u << 0u)), bool(tile_i & (1u << 1u)),
                                    bool(tile_i & (1u << 2u)), bool(tile_i & (1u << 3u)),
                                    bool(tile_i & (1u << 4u)), bool(tile_i & (1u << 5u)),
                                    bool(tile_i & (1u << 6u)), bool(tile_i & (1u << 7u))}),
            {tile_x, tile_y, 0});
    }
    common_data.rpgmaker_a2_all_tiles_mesh = common_data.builder.finish();

    enum class Demo : anton::u8 { sprite_types, lighting, count } demo = Demo::sprite_types;

    while (!window.should_close()) {
        wnd::poll_events();

        switch (demo) {
            case Demo::sprite_types: sprite_types_demo(common_data); break;
            case Demo::lighting: lighting_demo(common_data); break;
            default: break;
        }

        static bool pressed_space_before = input.is_pressed(wnd::InputKey::k_SPACE);
        bool pressed_space_now = input.is_pressed(wnd::InputKey::k_SPACE);
        if (!pressed_space_before && pressed_space_now) {
            demo = (Demo)(((anton::u8)demo) + 1u);
            if (demo == Demo::count)
                demo = (Demo)0;
        }
        pressed_space_before = pressed_space_now;
    }

    renderer.reset();
}