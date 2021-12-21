#pragma once

#include "../program.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace windfall {

struct Param {
  bool with_lighting = true;
  bool wireframe = false;
  bool use_zAtoon = false;
  bool no_texture = false;
  bool display_normals = false;
  bool use_shadow = true;
  bool use_cascaded_shadow = true;
  bool blend_between_cascade = true;
  int cascadeLevel = 0;
  bool cascade_show_layers = false;
  bool fitLightFrustrum = false;
  bool display_depth_map = false;
  bool peter_paning = false;
  bool pcf = true;
  int square_sample_size= 2;
  float shadow_bias = 0.125;
  std::vector<float> shadow_biases = {0.268f, 0.088f, 0.049f};
  float coeff_increase_shadow_frustum_z = 3.2f;
  float coeff_increase_shadow_frustum_xy = 1.2f;
  float near_plane_light = 20.0f, far_plane_light = 90.0f;
  float light_ambient = 0.7f;
  float light_diffuse = 0.8f;
  float light_pos[3] = {-21.0f, 49.0f, -29.0f}; //need a position for shadow
  float light_shadow_center[3] = {6.0f, 1.0f, -44.0f}; //The point the light will look at
  float angle_sun = 120.f;
  ImVec4 some_color = ImVec4(0.45f, 0.55f, 0.6f, 1.00f);
  float alpha_clip = 0.3f;
  float offset = 0.0f;
  float offset_water = 0.0f;
  float znear = 0.1f;
  float zfar = 600.f;
  float ortho_bounds[4] = {-50.0f, 50.0f, -60.0f, 70.0f};
  bool ortho_view = false;
  ImVec4 color_border = ImVec4(243.0f / 255.0f, 106.0f / 255.0f, 65.0f / 255.0f, 1.0f);
  ImVec4 color_center = ImVec4(246 / 255.0f, 197 / 255.0f, 193 / 255.0f, 1.0f);
  ImVec4 color_gradient = ImVec4(146 / 255.0f, 145 / 255.0f, 7 / 255.0f, 1.0f);
  float sun_magnification = 330.0f;
  float lowest_eye_cancer = 0.1f;
  bool hd = false;
  float wave_height = -10.0f;
  float sea_height = -10.f;
  float billboard_size[2] = {5.0f, 1.0f};
  program* program_windfall_with_lighting = nullptr;
} ;

void display(GLFWwindow* window, bool load_hd_texture, bool use_im_gui);
}