/* clang-format off */
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
/* clang-format on */

#include "aryibi/renderer.hpp"
#include "opengl/impl_types.hpp"

#include <anton/math/matrix4.hpp>
#include <anton/math/vector4.hpp>
#include <anton/math/transform.hpp>
#include <anton/math/vector2.hpp>

#include <iostream>
#include <memory>
#include <cstring> // For memcpy

namespace aryibi::renderer {

Renderer::~Renderer() = default;

Renderer::Renderer(GLFWwindow *_w)
    : window(_w), p_impl(std::make_unique<impl>()) {
  p_impl->lit_shader = ShaderHandle::from_file("assets/shaded_tile.vert",
                                               "assets/shaded_tile.frag");
  p_impl->unlit_shader = ShaderHandle::from_file("assets/basic_tile.vert",
                                                 "assets/basic_tile.frag");
  p_impl->lit_pal_shader = ShaderHandle::from_file(
      "assets/shaded_pal_tile.vert", "assets/shaded_pal_tile.frag");

  p_impl->depth_shader =
      ShaderHandle::from_file("assets/depth.vert", "assets/depth.frag");

  TextureHandle tex;
  constexpr u32 default_shadow_res_width = 1024;
  constexpr u32 default_shadow_res_height = 1024;
  tex.init(default_shadow_res_width, default_shadow_res_height,
           TextureHandle::ColorType::depth,
           TextureHandle::FilteringMethod::point);
  p_impl->shadow_depth_fb = Framebuffer(tex);

  p_impl->window_framebuffer.p_impl->handle = 0;
}

ShaderHandle Renderer::lit_shader() const { return p_impl->lit_shader; }
ShaderHandle Renderer::unlit_shader() const { return p_impl->unlit_shader; }
ShaderHandle Renderer::lit_paletted_shader() const {
  return p_impl->lit_pal_shader;
}

void Renderer::start_frame(Color clear_color) {
  // Start the ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(clear_color.fred(), clear_color.fgreen(), clear_color.fblue(), clear_color.falpha());
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::finish_frame() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(window);
}

Framebuffer Renderer::get_window_framebuffer() {
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  TextureHandle virtual_window_tex;
  virtual_window_tex.p_impl->width = display_w;
  virtual_window_tex.p_impl->height = display_h;
  virtual_window_tex.p_impl->filter = TextureHandle::FilteringMethod::point;
  virtual_window_tex.p_impl->color_type = TextureHandle::ColorType::rgba;
  p_impl->window_framebuffer.p_impl->tex = virtual_window_tex;
  p_impl->window_framebuffer.p_impl->handle = 0;
  return p_impl->window_framebuffer;
}

void Renderer::draw(DrawCmdList const &draw_commands,
                    Framebuffer const &output_fb) {
  glBindFramebuffer(GL_FRAMEBUFFER, p_impl->shadow_depth_fb.p_impl->handle);
  glClear(GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LEQUAL);
  glViewport(0, 0, p_impl->shadow_depth_fb.texture().width(),
             p_impl->shadow_depth_fb.texture().height());

  aml::Vector2 camera_view_size_in_tiles{
      (float)output_fb.texture().width() /
          draw_commands.camera.unit_size,
      (float)output_fb.texture().height() /
          draw_commands.camera.unit_size};
  aml::Matrix4 view =
      aml::inverse(aml::translate(draw_commands.camera.position));
  aml::Matrix4 proj;
  if (draw_commands.camera.center_view) {
    proj = aml::orthographic_rh(
        -camera_view_size_in_tiles.x / 2.f, camera_view_size_in_tiles.x / 2.f,
        -camera_view_size_in_tiles.y / 2.f, camera_view_size_in_tiles.y / 2.f,
        -20.0f, 20.0f);
  } else {
    proj = aml::orthographic_rh(0, camera_view_size_in_tiles.x,
                                -camera_view_size_in_tiles.y, 0, -20.0f, 20.0f);
  }
  // To create the light view, we position the light as if it were a camera and
  // then invert the matrix.
  aml::Matrix4 lightView = aml::translate(draw_commands.camera.position);
  // We want the light to be rotated on the Z and X axis to make it seem there's
  // some directionality to it.
  // TODO: Move direction data to DrawCmd
  lightView *=
      aml::rotate_z(-aml::pi / 5.f) * aml::rotate_x(-aml::pi / 5.f) * aml::scale(2.f);
  lightView = aml::inverse(lightView);
  aml::Matrix4 lightSpaceMatrix = proj * lightView;

  glUseProgram(p_impl->depth_shader.p_impl->handle);
  for (const auto &cmd : draw_commands.commands) {
    if (!cmd.cast_shadows)
      continue;
    aml::Matrix4 model = aml::translate(cmd.transform.position);

    glBindVertexArray(cmd.mesh.p_impl->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cmd.texture.p_impl->handle);
    glUniformMatrix4fv(0, 1, GL_FALSE, model.get_raw()); // Model matrix
    glUniformMatrix4fv(3, 1, GL_FALSE,
                       lightSpaceMatrix.get_raw()); // Light space matrix
    glDrawArrays(GL_TRIANGLES, 0, cmd.mesh.p_impl->vertex_count);
  }
  glViewport(0, 0, output_fb.texture().width(), output_fb.texture().height());
  glBindFramebuffer(GL_FRAMEBUFFER, output_fb.p_impl->handle);
  for (const auto &cmd : draw_commands.commands) {
    bool is_lit =
        cmd.shader.p_impl->shadow_tex_location != static_cast<u32>(-1);
    bool is_paletted =
        cmd.shader.p_impl->palette_tex_location != static_cast<u32>(-1);
    aml::Matrix4 model = aml::translate(cmd.transform.position);

    glUseProgram(cmd.shader.p_impl->handle);
    glUniformMatrix4fv(0, 1, GL_FALSE, model.get_raw()); // Model matrix
    glUniformMatrix4fv(1, 1, GL_FALSE, proj.get_raw());  // Projection matrix
    glUniformMatrix4fv(2, 1, GL_FALSE, view.get_raw());  // View matrix
    if (is_lit)
      glUniformMatrix4fv(3, 1, GL_FALSE,
                         lightSpaceMatrix.get_raw()); // Light space matrix
    glBindVertexArray(cmd.mesh.p_impl->vao);

    glUniform1i(cmd.shader.p_impl->tile_tex_location,
                0); // Set tile sampler2D to GL_TEXTURE0
    glUniform1i(cmd.shader.p_impl->shadow_tex_location,
                1); // Set shadow sampler2D to GL_TEXTURE1
    glUniform1i(cmd.shader.p_impl->palette_tex_location,
                2); // Set palette sampler2D to GL_TEXTURE2
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cmd.texture.p_impl->handle);
    if (is_lit) {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D,
                    p_impl->shadow_depth_fb.texture().p_impl->handle);
    }
    if (is_paletted) {
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, p_impl->palette_texture.p_impl->handle);
    }
    glUseProgram(cmd.shader.p_impl->handle);

    // Light space matrix is already set if used
    glDrawArrays(GL_TRIANGLES, 0, cmd.mesh.p_impl->vertex_count);
  }
}

void Renderer::clear(Framebuffer &fb, aml::Vector4 color) {
  glBindFramebuffer(GL_FRAMEBUFFER, fb.p_impl->handle);
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::set_palette(ColorPalette const &palette) {
  p_impl->palette_texture.unload();
  // The palette texture is a regular 2D texture. The X axis represents the
  // different shades of a color, and the Y axis represents the different colors
  // available. We are going to dedicate 0,0 and its row entirely just for the
  // transparent color, So we'll add 1 to the height.
  const u32 palette_texture_height = palette.colors.size() + 1;
  // The width of the palette texture will be equal to the maximum shades
  // available of any color.
  u32 palette_texture_width = 0;
  for (const auto &color : palette.colors) {
    if (color.shades.size() > palette_texture_width)
      palette_texture_width = color.shades.size();
  }
  assert(palette_texture_height > 0 && palette_texture_width > 0);

  constexpr int bytes_per_pixel = 4;
  auto palette_texture_data =
      new unsigned char[palette_texture_width * palette_texture_height *
                        bytes_per_pixel];
  std::memcpy((void *)(palette_texture_data),
              (void *)&palette.transparent_color, sizeof(u32));
  int px_y = 1, px_x = 0;
  for (const auto &color : palette.colors) {
    for (const auto &shade : color.shades) {
      std::memcpy(
          (void *)(palette_texture_data +
                   (px_x + px_y * palette_texture_width) * bytes_per_pixel),
          (void *)&shade, sizeof(u32));
      ++px_x;
    }
    px_x = 0;
    ++px_y;
  }
  p_impl->palette_texture.init(palette_texture_width, palette_texture_height,
                               TextureHandle::ColorType::rgba,
                               TextureHandle::FilteringMethod::point,
                               palette_texture_data);
  delete[] palette_texture_data;
}

} // namespace aryibi::renderer