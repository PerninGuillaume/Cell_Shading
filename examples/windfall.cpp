#include "windfall.h"

#include <iostream>
#include <memory>

#include "../Camera.h"
#include "../callback.h"
#include "../Skybox.h"
#include "../Helper.h"
#include "../Model.h"
#include "../misc.h"
#include "../Shadow.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace windfall {

bool use_im_gui = true;
float lastFrame = 0.0f;

std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//Setup of different default values
struct {
  bool with_lighting = true;
  bool wireframe = false;
  bool use_zAtoon = false;
  bool no_texture = false;
  bool display_normals = false;
  bool flat_look = false;
  bool use_shadow = true;
  bool display_depth_map = false;
  bool peter_paning = false;
  bool pcf = true;
  float shadow_bias = 0.005f;
  float near_plane_light = 20.0f, far_plane_light = 90.0f;
  float light_ambient = 0.7f;
  float light_diffuse = 0.8f;
  float light_dir[3] = {-0.3f, -0.7f, -0.3f};
  float light_pos[3] = {-21.0f, 49.0f, -29.0f}; //need a position for shadow
  float light_shadow_center[3] = {6.0f, 1.0f, -44.0f}; //The point the light will look at
  ImVec4 some_color = ImVec4(0.45f, 0.55f, 0.6f, 1.00f);
  float alpha_clip = 0.3f;
  float offset = 0.0f;
  float offset_water = 0.0f;
  float ortho_bounds[4] = {-50.0f, 50.0f, -60.0f, 70.0f};
  bool ortho_view = false;
  ImVec4 color_border = ImVec4(243.0f / 255.0f, 106.0f / 255.0f, 65.0f / 255.0f, 1.0f);
  ImVec4 color_center = ImVec4(246 / 255.0f, 197 / 255.0f, 193 / 255.0f, 1.0f);
  //ImVec4 color_inner_ring = ImVec4(1.0f, 1.0f, 0.9f, 1.0f);
  ImVec4 color_gradient = ImVec4(146 / 255.0f, 145 / 255.0f, 7 / 255.0f, 1.0f);
  float sun_magnification = 330.0f;
} params;

void set_im_gui_options() {

  if (use_im_gui) {
    ImGui::Begin("Windfall options");
    if (ImGui::TreeNode("Sun")) {
      ImGui::ColorPicker3("Border", (float*)&params.color_border);
      ImGui::ColorPicker3("Center", (float*)&params.color_center);
      //ImGui::ColorPicker3("Inner ring", (float*)&params.color_inner_ring);
      ImGui::ColorPicker3("Gradient", (float*)&params.color_gradient);
      ImGui::SliderFloat("Sun magnification", &params.sun_magnification, 0.0f, 1000.0f);

      ImGui::TreePop();
      ImGui::Separator();
    }
    if (ImGui::TreeNode("Lighting")) {
      ImGui::Checkbox("Enable lighting", &params.with_lighting);
      ImGui::SliderFloat("Light diffuse", &params.light_diffuse, 0.0f, 1.0f);
      ImGui::SliderFloat("Light ambient", &params.light_ambient, 0.0f, 1.0f);  ImGui::Checkbox("Flat look", &params.flat_look);
      ImGui::Checkbox("Use Zatoon", &params.use_zAtoon);

      ImGui::TreePop();
      ImGui::Separator();
    }

    if (ImGui::TreeNode("Shadow")) {
      ImGui::Checkbox("Shadow", &params.use_shadow);
      ImGui::Checkbox("Depth texture", &params.display_depth_map);
      ImGui::Checkbox("Peter Paning", &params.peter_paning);
      ImGui::Checkbox("PCF", &params.pcf);
      ImGui::SliderFloat("Shadow bias", &params.shadow_bias, 0.0f, 0.1f);
      ImGui::SliderFloat("Near plane light frustrum", &params.near_plane_light, 0.0f, 100.0f);
      ImGui::SliderFloat("Far plane light frustrum", &params.far_plane_light, 10.0f, 100.0f);
      ImGui::SliderFloat4("Ortho bounds", params.ortho_bounds, -100.0f, 100.0f);
      ImGui::Checkbox("Ortho view", &params.ortho_view);
      ImGui::SliderFloat3("Light position", params.light_pos, -100.0f, 100.0f);
      ImGui::SliderFloat3("Light shadow center", params.light_shadow_center, -100.0f, 100.0f);
      ImGui::SliderFloat3("Light direction", params.light_dir, -1.0f, 1.0f);
      ImGui::TreePop();
      ImGui::Separator();
    }
    ImGui::Checkbox("WireFrame", &params.wireframe);
    ImGui::Checkbox("No texture", &params.no_texture);
    ImGui::Checkbox("Display Normals", &params.display_normals);
    ImGui::SliderFloat("Alpha clip", &params.alpha_clip, 0.0f, 1.0f);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
}
unsigned int water_create_VAO() {
  float heightf = -11;
  float waterVertices[] = {
      -20000.0f, heightf, -20000.0f,
      20000.0f, heightf, -20000.0f,
      20000.0f, heightf, 20000.0f,
      20000.0f, heightf, 20000.0f,
      -20000.0f, heightf, 20000.0f,
      -20000.0f, heightf, -20000.0f,};

  unsigned int waterVAO, waterVBO;
  glGenVertexArrays(1, &waterVAO);
  glGenBuffers(1, &waterVBO);
  glBindVertexArray(waterVAO);
  glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), &waterVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

  return waterVAO;
}

std::vector<unsigned int> loadSunTextures()
{
  std::vector<unsigned int> sun_textures{};

  std::vector<std::string> files {
     "images/sprites/sun_border.png",
     "images/sprites/sun_eclat.png",
      "images/sprites/sun_center.png",
      "images/sprites/sun_inner_ring.png",
      "images/sprites/sun_gradient.png"
  };

for (const auto &file : files) {
    sun_textures.emplace_back(load_image(file));
  }
  return sun_textures;
}

std::vector<unsigned int> loadShore()
{
  std::vector<unsigned int> shore{};

  std::vector<std::string> files {
                                  "images/sprites/shore_alpha_wave.png",
                                  "images/sprites/shore_black_wave.png",
                                  "images/sprites/shore_limit_wave.png",
                                  "images/sprites/shore_mask.png"};

  for (const auto &file : files)
  {
    shore.emplace_back(load_image(file));
  }
  return shore;
}

unsigned int shore_create_VAO(int &nb_of_points) {

  /*std::vector<glm::vec3> points = {
      glm::vec3 (-2042.0f, 0.0f, 2800.0f),
      glm::vec3 (-2822.0f, 0.0f, 2894.0f),
      glm::vec3 (-2822.0f, 0.0f, 3107.0f),
      glm::vec3 (-2042.0f, 0.0f, 3012.0f),

      glm::vec3(-1347.0f, 0.0f, 2318.0f),
      glm::vec3 (-2042.0f, 0.0f, 2800.0f),
      glm::vec3 (-2042.0f, 0.0f, 3012.0f),
      glm::vec3(-1347.0f, 0.0f, 2529.0f),

      glm::vec3(-1186.0f, 0.0f, 2406.0f),
      glm::vec3(-1347.0f, 0.0f, 2318.0f),
      glm::vec3(-1347.0f, 0.0f, 2529.0f),
      glm::vec3(-1187.0f, 0.0f, 2618.0f)

  };*/

  std::vector<glm::vec3> points = {
      glm::vec3 (-2822.0f, 0.0f, 2894.0f),
      glm::vec3 (-2822.0f, 0.0f, 3107.0f),

      glm::vec3 (-2042.0f, 0.0f, 2800.0f),
      glm::vec3 (-2042.0f, 0.0f, 3012.0f),

      glm::vec3(-1347.0f, 0.0f, 2318.0f),
      glm::vec3(-1347.0f, 0.0f, 2529.0f),

      glm::vec3(-1186.0f, 0.0f, 2406.0f),
      glm::vec3(-1187.0f, 0.0f, 2618.0f),

      glm::vec3(-969.0f, 0.0f, 2471.0f),
      glm::vec3(-969.0f, 0.0f, 2683.0f),

      glm::vec3(-703.0f, 0.0f, 2503.0f),
      glm::vec3(-703.0f, 0.0f, 2713.0f),

      glm::vec3(-483.0f, 0.0f, 2417.0f),
      glm::vec3(-483.0f, 0.0f, 2629.0f),

      glm::vec3(584.0f, 0.0f, 1704.0f),
      glm::vec3(584.0f, 0.0f, 2000.0f),

      glm::vec3(884.0f, 0.0f, 2151.0f),
      glm::vec3(884.0f, 0.0f, 2362.0f),

      glm::vec3(1420.0f, 0.0f, 1711.0f),
      glm::vec3(1420.0f, 0.0f, 1975.0f),

      glm::vec3(1532.0f, 0.0f, 1953.0f),
      glm::vec3(1532.0f, 0.0f, 2216.0f),

      glm::vec3(1758.0f, 0.0f, 1954.0f),
      glm::vec3(1758.0f, 0.0f, 2196.0f),

      glm::vec3(1977.0f, 0.0f, 1747.0f),
      glm::vec3(1977.0f, 0.0f, 2013.0f),

      glm::vec3(2086.0f, 0.0f, 1380.0f),
      glm::vec3(2347.0f, 0.0f, 1380.0f),

      glm::vec3(3156.0f, 0.0f, -157.0f),
      glm::vec3(3156.0f, 0.0f, 337.0f),

      glm::vec3(3711.0f, 0.0f, -1712.0f),
      glm::vec3(4075.0f, 0.0f, -1712.0f),

      glm::vec3(4118.0f, 0.0f, -2054.0f),
      glm::vec3(4280.0f, 0.0f, -1899.0f),

      glm::vec3(3736.0f, 0.0f, -2918.0f),
      glm::vec3(4035.0f, 0.0f, -2918.0f),

      glm::vec3(3194.0f, 0.0f, -3731.0f),
      glm::vec3(3366.0f, 0.0f, -3985.0f),

      glm::vec3(3166.0f, 0.0f, -3983.0f),
      glm::vec3(3344.0f, 0.0f, -4331.0f),

      glm::vec3(2600.0f, 0.0f, -4128.0f),
      glm::vec3(2641.0f, 0.0f, -4227.0f),

      glm::vec3(2270.0f, 0.0f, -4547.0f),
      glm::vec3(2392.0f, 0.0f, -4618.0f),

      glm::vec3(1903.0f, 0.0f, -4739.0f),
      glm::vec3(1984.0f, 0.0f, -4857.0f),

      glm::vec3(1629.0f, 0.0f, -5083.0f),
      glm::vec3(1694.0f, 0.0f, -5236.0f),

      glm::vec3(885.0f, 0.0f, -5535.0f),
      glm::vec3(917.0f, 0.0f, -5640.0f),

      glm::vec3(615.0f, 0.0f, -5622.0f),
      glm::vec3(572.0f, 0.0f, -5673.0f),

      glm::vec3(288.0f, 0.0f, -5345.0f),
      glm::vec3(199.0f, 0.0f, -5405.0f),

      glm::vec3(-663.0f, 0.0f, -4879.0f),
      glm::vec3(-716.0f, 0.0f, -4933.0f),

      glm::vec3(-964.0f, 0.0f, -4576.0f),
      glm::vec3(-1011.0f, 0.0f, -4594.0f),

      glm::vec3(-918.0f, 0.0f, -4354.0f),
      glm::vec3(-971.0f, 0.0f, -4359.0f),

      glm::vec3(-1102.0f, 0.0f, -4033.0f),
      glm::vec3(-1189.0f, 0.0f, -4023.0f),

      glm::vec3(-843.0f, 0.0f, -3585.0f),
      glm::vec3(-925.0f, 0.0f, -3611.0f),

      glm::vec3(-1103.0f, 0.0f, -3357.0f),
      glm::vec3(-1135.0f, 0.0f, -3388.0f),

      glm::vec3(-1189.0f, 0.0f, -3218.0f),
      glm::vec3(-1219.0f, 0.0f, -3264.0f),

      glm::vec3(-1390.0f, 0.0f, -3175.0f),
      glm::vec3(-1409.0f, 0.0f, -3235.0f),

      glm::vec3(-1657.0f, 0.0f, -3035.0f),
      glm::vec3(-1665.0f, 0.0f, -3101.0f),

      glm::vec3(-1766.0f, 0.0f, -3057.0f),
      glm::vec3(-1768.0f, 0.0f, -3098.0f),

      glm::vec3(-1906.0f, 0.0f, -3026.0f),
      glm::vec3(-1946.0f, 0.0f, -3065.0f),

      glm::vec3(-1960.0f, 0.0f, -2852.0f),
      glm::vec3(-2049.0f, 0.0f, -2868.0f),

      glm::vec3(-1918.0f, 0.0f, -2678.0f),
      glm::vec3(-1966.0f, 0.0f, -2641.0f),

      glm::vec3(-1680.0f, 0.0f, -2479.0f),
      glm::vec3(-1746.0f, 0.0f, -2408.0f),

      glm::vec3(-1547.0f, 0.0f, -2041.0f),
      glm::vec3(-1617.0f, 0.0f, -2083.0f),

      glm::vec3(-1819.0f, 0.0f, -1954.0f),
      glm::vec3(-1816.0f, 0.0f, -2052.0f),

      glm::vec3(-2133.0f, 0.0f, -2273.0f),
      glm::vec3(-2082.0f, 0.0f, -2317.0f),

      glm::vec3(-2232.0f, 0.0f, -2469.0f),
      glm::vec3(-2196.0f, 0.0f, -2520.0f),

      glm::vec3(-2404.0f, 0.0f, -2501.0f),
      glm::vec3(-2403.0f, 0.0f, -2563.0f),

      glm::vec3(-2657.0f, 0.0f, -2441.0f),
      glm::vec3(-2702.0f, 0.0f, -2484.0f),

      glm::vec3(-2771.0f, 0.0f, -2142.0f),
      glm::vec3(-2860.0f, 0.0f, -2147.0f),

      glm::vec3(-2767.0f, 0.0f, -1829.0f),
      glm::vec3(-2815.0f, 0.0f, -1880.0f),

      glm::vec3(-2934.0f, 0.0f, -1646.0f),
      glm::vec3(-2967.0f, 0.0f, -1719.0f),

      glm::vec3(-3365.0f, 0.0f, -1463.0f),
      glm::vec3(-3390.0f, 0.0f, -1572.0f),

      glm::vec3(-3858.0f, 0.0f, -1089.0f),
      glm::vec3(-3927.0f, 0.0f, -1187.0f),

      glm::vec3(-4240.0f, 0.0f, -649.0f),
      glm::vec3(-4324.0f, 0.0f, -684.0f),

      glm::vec3(-4558.0f, 0.0f, -8.0f),
      glm::vec3(-4758.0f, 0.0f, 15.0f),

      glm::vec3(-4612.0f, 0.0f, 641.0f),
      glm::vec3(-4689.0f, 0.0f, 636.0f),

      glm::vec3(-4418.0f, 0.0f, 1273.0f),
      glm::vec3(-4513.0f, 0.0f, 1264.0f),

      glm::vec3(-4729.0f, 0.0f, 1719.0f),
      glm::vec3(-4769.0f, 0.0f, 1709.0f),

      glm::vec3(-4718.0f, 0.0f, 2232.0f),
      glm::vec3(-4754.0f, 0.0f, 2239.0f),

      glm::vec3(-4427.0f, 0.0f, 2208.0f),
      glm::vec3(-4408.0f, 0.0f, 2245.0f),

      glm::vec3(-4020.0f, 0.0f, 2213.0f),
      glm::vec3(-4048.0f, 0.0f, 2276.0f),

      glm::vec3 (-2822.0f, 0.0f, 2894.0f),
      glm::vec3 (-2822.0f, 0.0f, 3107.0f)


  };

  std::vector<glm::vec3> points_2 {
    glm::vec3(-2824.0f, 0.0f, -3399.0f),
    glm::vec3(-2879.0f, 0.0f, -3401.0f),

    glm::vec3(-2836.0f, 0.0f, -3218.0f),
    glm::vec3(-2948.0f, 0.0f, -3081.0f),

    glm::vec3(-2717.0f, 0.0f, -3168.0f),
    glm::vec3(-2727.0f, 0.0f, -3118.0f),

    glm::vec3(-2430.0f, 0.0f, -3247.0f),
    glm::vec3(-2360.0f, 0.0f, -3215.0f),

    glm::vec3(-2413.0f, 0.0f, -3386.0f),
    glm::vec3(-2333.0f, 0.0f, -3379.0f),

    glm::vec3(-2583.0f, 0.0f, -3556.0f),
    glm::vec3(-2590.0f, 0.0f, -3607.0f),

    glm::vec3(-2824.0f, 0.0f, -3399.0f),
    glm::vec3(-2879.0f, 0.0f, -3401.0f)

  };

  std::vector<glm::vec3> points_3 = {
      glm::vec3(-1357.0f, 0.0f, -5879.0f),
      glm::vec3(-1445.0f, 0.0f, -5955.0f),

      glm::vec3(-1416.0f, 0.0f, -5687.0f),
      glm::vec3(-1461.0f, 0.0f, -5676.0f),

      glm::vec3(-1379.0f, 0.0f, -5610.0f),
      glm::vec3(-1400.0f, 0.0f, -5548.0f),

      glm::vec3(-1055.0f, 0.0f, -5675.0f),
      glm::vec3(-1016.0f, 0.0f, -5669.0f),

      glm::vec3(-1177.0f, 0.0f, -5847.0f),
      glm::vec3(-1156.0f, 0.0f, -5887.0f),

      glm::vec3(-1357.0f, 0.0f, -5879.0f),
      glm::vec3(-1445.0f, 0.0f, -5955.0f)
  };

  std::vector<glm::vec3> points_4 = {
      glm::vec3(2321.0f, 0.0f, 2586.0f),
      glm::vec3(2188.0f, 0.0f, 2636.0f),

      glm::vec3(2639.0f, 0.0f, 2628.0f),
      glm::vec3(2756.0f, 0.0f, 2671.0f),

      glm::vec3(2598.0f, 0.0f, 2254.0f),
      glm::vec3(2655.0f, 0.0f, 2200.0f),

      glm::vec3(2370.0f, 0.0f, 2258.0f),
      glm::vec3(2259.0f, 0.0f, 2199.0f),

      glm::vec3(2321.0f, 0.0f, 2586.0f),
      glm::vec3(2188.0f, 0.0f, 2636.0f)
  };
  nb_of_points = (points.size() / 2) - 1 + (points_2.size() / 2) - 1 + (points_3.size() / 2) - 1 + (points_4.size() / 2) - 1;
  glm::mat4 model_mat_windfall = glm::mat4(1.0f);
  model_mat_windfall = glm::translate(model_mat_windfall, glm::vec3(1.0f, -10.0f, -25.0f));
  model_mat_windfall = glm::scale(model_mat_windfall, glm::vec3(0.01f, 0.01f, 0.01f));
  for (int i = 0; i < points.size(); ++i) {
    points[i] = model_mat_windfall * glm::vec4(points[i], 1.0f);
  }
  for (int i = 0; i < points_2.size(); ++i) {
    points_2[i] = model_mat_windfall * glm::vec4(points_2[i], 1.0f);
  }
  for (int i = 0; i < points_3.size(); ++i) {
    points_3[i] = model_mat_windfall * glm::vec4(points_3[i], 1.0f);
  }
  for (int i = 0; i < points_4.size(); ++i) {
    points_4[i] = model_mat_windfall * glm::vec4(points_4[i], 1.0f);
  }


  float heightf = -10.05f;
  std::vector<float> shoreVertices = {};
  for (int i = 0; i < points.size() - 2; i += 2) {
    std::vector<float> shoreVertices_quad = {
        points[i].x, heightf, points[i].z, 0.0f, 0.0f,
        points[i + 1].x, heightf, points[i + 1].z, 0.0f, 1.0f,
        points[i + 2].x, heightf, points[i + 2].z, 1.0f, 0.0f,

        points[i + 3].x, heightf, points[i + 3].z, 1.0f, 1.0f,
        points[i + 2].x, heightf, points[i + 2].z, 1.0f, 0.0f,
        points[i + 1].x, heightf, points[i + 1].z, 0.0f, 1.0f
    };
    shoreVertices.insert(shoreVertices.end(), shoreVertices_quad.begin(), shoreVertices_quad.end());
  }
  for (int i = 0; i < points_2.size() - 2; i += 2) {
    std::vector<float> shoreVertices_quad = {
        points_2[i].x, heightf, points_2[i].z, 0.0f, 0.0f,
        points_2[i + 1].x, heightf, points_2[i + 1].z, 0.0f, 1.0f,
        points_2[i + 2].x, heightf, points_2[i + 2].z, 1.0f, 0.0f,

        points_2[i + 3].x, heightf, points_2[i + 3].z, 1.0f, 1.0f,
        points_2[i + 2].x, heightf, points_2[i + 2].z, 1.0f, 0.0f,
        points_2[i + 1].x, heightf, points_2[i + 1].z, 0.0f, 1.0f
    };
    shoreVertices.insert(shoreVertices.end(), shoreVertices_quad.begin(), shoreVertices_quad.end());
  }
  for (int i = 0; i < points_3.size() - 2; i += 2) {
    std::vector<float> shoreVertices_quad = {
        points_3[i].x, heightf, points_3[i].z, 0.0f, 0.0f,
        points_3[i + 1].x, heightf, points_3[i + 1].z, 0.0f, 1.0f,
        points_3[i + 2].x, heightf, points_3[i + 2].z, 1.0f, 0.0f,

        points_3[i + 3].x, heightf, points_3[i + 3].z, 1.0f, 1.0f,
        points_3[i + 2].x, heightf, points_3[i + 2].z, 1.0f, 0.0f,
        points_3[i + 1].x, heightf, points_3[i + 1].z, 0.0f, 1.0f
    };
    shoreVertices.insert(shoreVertices.end(), shoreVertices_quad.begin(), shoreVertices_quad.end());
  }
  for (int i = 0; i < points_4.size() - 2; i += 2) {
    std::vector<float> shoreVertices_quad = {
        points_4[i].x, heightf, points_4[i].z, 0.0f, 0.0f,
        points_4[i + 1].x, heightf, points_4[i + 1].z, 0.0f, 1.0f,
        points_4[i + 2].x, heightf, points_4[i + 2].z, 1.0f, 0.0f,

        points_4[i + 3].x, heightf, points_4[i + 3].z, 1.0f, 1.0f,
        points_4[i + 2].x, heightf, points_4[i + 2].z, 1.0f, 0.0f,
        points_4[i + 1].x, heightf, points_4[i + 1].z, 0.0f, 1.0f
    };
    shoreVertices.insert(shoreVertices.end(), shoreVertices_quad.begin(), shoreVertices_quad.end());
  }
  unsigned int shoreVAO, shoreVBO;
  glGenVertexArrays(1, &shoreVAO);
  glGenBuffers(1, &shoreVBO);
  glBindVertexArray(shoreVAO);
  glBindBuffer(GL_ARRAY_BUFFER, shoreVBO);
  glBufferData(GL_ARRAY_BUFFER, shoreVertices.size() * sizeof(float), shoreVertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  return shoreVAO;
}

unsigned int sun_create_VAO() {
  float x_start = -50.0f;
  float y_start = -50.0f;
  float x_end = 50.0f;
  float y_end = 50.0f;
  float height = 500.0f;
  float sunVertices[] = {
      x_end,  height, y_end,  0.0f, 0.0f,
      x_start, height, y_end,     1.0f, 0.0f,
      x_start, height,  y_start,      1.0f, 1.0f,

      x_start,  height, y_start,  1.0f, 1.0f,
      x_end, height,  y_start,      0.0f, 1.0f,
      x_end, height, y_end,     0.0f, 0.0f,
  };
  unsigned int sunVAO, sunVBO;
  glGenVertexArrays(1, &sunVAO);
  glGenBuffers(1, &sunVBO);
  glBindVertexArray(sunVAO);
  glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(sunVertices), &sunVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  return sunVAO;

}

void display(GLFWwindow *window) {
  int SRC_WIDTH, SRC_HEIGHT;
  glfwGetWindowSize(window, &SRC_WIDTH, &SRC_HEIGHT);
  //Capture the mouse
  if (use_im_gui) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
  }
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  // Setup Platfrom/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 450");
  ImGui::StyleColorsDark();

  program *shadow_shader_depth = init_program("shaders/vertex_shadow_depth.glsl",
                                              "shaders/fragment_shadow_depth.glsl");
  program *quad_depth_shader = init_program("shaders/vertex_normalized_coord.glsl", "shaders/fragment_quad_depth.glsl");
  program *shader_normals = init_program("shaders/vertex_normals.glsl", "shaders/fragment_normals.glsl",
                                         "shaders/geometry_normals.glsl");
  program *program_windfall;
  program *program_windfall_without_lighting = init_program("shaders/vertex_model.glsl",
                                  "shaders/fragment_model.glsl");
  program *program_windfall_with_lighting = init_program("shaders/vertex_link.glsl",
                                           "shaders/fragment_link.glsl");
  program_windfall = program_windfall_with_lighting;
  program *program_skybox = init_program("shaders/vertex_skybox.glsl",
                                         "shaders/fragment_skybox.glsl");
  program *program_water = init_program("shaders/vertex_water.glsl",
                                        "shaders/fragment_water.glsl");
  program *program_clouds = init_program("shaders/vertex_clouds.glsl",
                                         "shaders/fragment_clouds.glsl");
  program *program_shore = init_program("shaders/vertex_shore.glsl",
                                        "shaders/fragment_shore.glsl");
  program *program_sun = init_program("shaders/vertex_sun.glsl",
                                         "shaders/fragment_sun.glsl");
  //stbi_set_flip_vertically_on_load(true);
  Model windfall_flat("models/Windfall Island/Windfall/Windfall_save.obj");
  Model windfall_smooth("models/Windfall Island/Windfall/Windfall.obj");

  unsigned int size_shadow_texture = 4096;
  Shadow shadow = Shadow(size_shadow_texture, size_shadow_texture);

  glEnable(GL_DEPTH_TEST);

  //Skybox

  unsigned int cubemapTexture = loadSkyBox(program_skybox);
  unsigned int skyboxVAO = skyBox_create_VAO();

  // Water

  unsigned int waterVAO = water_create_VAO();

  std::vector<unsigned int> shoreTextures = loadShore();
  int nb_of_waves;
  unsigned int shoreVAO = shore_create_VAO(nb_of_waves);

  //Zatoon
  set_zAtoon(program_windfall);

  // Sun
  std::vector<unsigned int> sun_textures = loadSunTextures();
  unsigned int sunVAO = sun_create_VAO();


  Helper helper = Helper(camera, use_im_gui);

  // Clouds
  std::vector<unsigned int> cloudsTextures = loadClouds();
  unsigned int cloudsVAO = clouds_create_VAO();


  while (!glfwWindowShouldClose(window)) {

    if (use_im_gui) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
    }

    float currentFrame = glfwGetTime();
    helper.deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwSetWindowUserPointer(window, &helper);
    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera->view_matrix();
    glm::mat4 projection;
    if (!params.ortho_view) {
      projection = glm::perspective(glm::radians(camera->fov_camera), 800.0f / 600.0f,
                                              0.1f, 1000.0f);
    } else {
      projection = glm::ortho(params.ortho_bounds[0], params.ortho_bounds[1], params.ortho_bounds[2], params.ortho_bounds[3], params.near_plane_light, params.far_plane_light);
    }




    //--------------------Windfall rendering-----------------------

    if (params.wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (params.with_lighting) {
      program_windfall = program_windfall_with_lighting;
    } else {
      program_windfall = program_windfall_without_lighting;
    }


    // 1. Render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    //lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane_light, far_plane_light);
    lightProjection = glm::ortho(params.ortho_bounds[0], params.ortho_bounds[1], params.ortho_bounds[2], params.ortho_bounds[3], params.near_plane_light, params.far_plane_light);
    glm::vec3 eye = glm::vec3(params.light_pos[0], params.light_pos[1], params.light_pos[2]);
    glm::vec3 center = glm::vec3(params.light_shadow_center[0], params.light_shadow_center[1], params.light_shadow_center[2]);
    glm::vec3 up = glm::vec3(0.6f, 0.02f, -0.77f);
    //glm::vec3 center = glm::vec3(0.0f);
    lightView = glm::lookAt(eye,
                            center,
                            up);
    lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, shadow.shadow_width, shadow.shadow_height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 model_mat_windfall = glm::mat4(1.0f);
    model_mat_windfall = glm::translate(model_mat_windfall, glm::vec3(1.0f, -10.0f, -25.0f));
    model_mat_windfall = glm::scale(model_mat_windfall, glm::vec3(0.01f, 0.01f, 0.01f));


    shadow_shader_depth->set_uniform_mat4("model", model_mat_windfall);
    shadow_shader_depth->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);

    if (params.peter_paning) {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
    }
    windfall_smooth.draw(shadow_shader_depth); //The choice of the normal has no influence on the depth
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // reset viewport

    glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. render scene as normal using the generated depth/shadow map
    // --------------------------------------------------------------


    program_windfall->set_uniform_mat4("model", model_mat_windfall);
    program_windfall->set_uniform_mat4("view", view);
    program_windfall->set_uniform_mat4("projection", projection);
    program_windfall->set_uniform_float("alpha_clip", params.alpha_clip);
    program_windfall->set_uniform_bool("use_zAtoon", params.use_zAtoon);
    program_windfall->set_uniform_bool("no_texture", params.no_texture);
    program_windfall->set_uniform_bool("use_shadow", params.use_shadow);
    program_windfall->set_uniform_bool("pcf", params.pcf);
    program_windfall->set_uniform_float("shadow_bias", params.shadow_bias);

    program_windfall->set_uniform_vec3("dirLight.direction", params.light_dir[0], params.light_dir[1], params.light_dir[2]);

    program_windfall->set_uniform_vec3("dirLight.ambient", params.light_ambient);
    program_windfall->set_uniform_vec3("dirLight.diffuse", params.light_diffuse);

    program_windfall->set_uniform_int("shadowMap", 1);
    program_windfall->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);

    if (params.flat_look)
      windfall_flat.draw(program_windfall);
    else
      windfall_smooth.draw(program_windfall);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

//------------------ Water rendering-----------------------------

    program_water->use();
    glm::mat4 model_mat_water = glm::mat4(1.0f);
    program_water->set_uniform_mat4("view", view);
    program_water->set_uniform_mat4("projection", projection);
    program_water->set_uniform_mat4("model", model_mat_water);


    glBindVertexArray(waterVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);




//-----------------------Normals------------------------------------
    if (params.display_normals) {
      shader_normals->set_uniform_mat4("projection", projection);
      shader_normals->set_uniform_mat4("view", view);
      shader_normals->set_uniform_mat4("model", model_mat_windfall);
      if (params.flat_look)
        windfall_flat.draw(shader_normals);
      else
        windfall_smooth.draw(shader_normals);
    }

//--------------------------Skybox rendering---------------------------
    program_skybox->use();

    glDepthFunc(GL_LEQUAL);
    view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
    program_skybox->set_uniform_mat4("view", view);
    program_skybox->set_uniform_mat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);


    //------------------- Shore rendering --------------------------


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program_shore->use();

    program_shore->set_uniform_int("tex_alpha_wave", 0);
    program_shore->set_uniform_int("tex_black_wave", 1);
    program_shore->set_uniform_int("tex_limit_wave", 2);
    program_shore->set_uniform_int("tex_mask_wave", 3);

    view = camera->view_matrix();
    if (params.offset_water > 1.0)
      params.offset_water = -1.0;
    program_shore->set_uniform_float("offset", params.offset_water);

    glm::mat4 model_mat_shore = glm::mat4(1.0f);
//    view = glm::translate(view, glm::vec3(-offset * 10));
    program_shore->set_uniform_mat4("view", view);
    program_shore->set_uniform_mat4("projection", projection);
    program_shore->set_uniform_float("alpha_clip", params.alpha_clip);
//    model_mat_shore = glm::rotate(model_mat_shore, (float)PI / 4.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    program_shore->set_uniform_mat4("model", model_mat_shore);
    params.offset_water += helper.deltaTime;
//    std::cout <<


    glBindVertexArray(shoreVAO);
    for (int i = 0; i < shoreTextures.size(); ++i) {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, shoreTextures[i]);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6 * nb_of_waves);

    //------------------ Sun rendering --------------------------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glm::mat4 model_mat_sun = glm::mat4(1.0f);
    model_mat_sun = glm::scale(model_mat_sun, glm::vec3(1.0f));
    program_sun->set_uniform_mat4("model", model_mat_sun);
    glm::mat4 view_mat_sun = glm::mat4(glm::mat3(camera->view_matrix()));

    float alignment = glm::dot(camera->front, glm::vec3(0.0f, 1.0f, 0.0f));

    float alignment_limit = 0.9f;
    if (alignment > alignment_limit) {
      float translation = -(alignment - alignment_limit) * 10 * params.sun_magnification;
      view_mat_sun = glm::translate(view_mat_sun, glm::vec3(0.0f, translation, 0.0f));
    }
    //glm::mat4 view_mat_sun = camera->view_matrix();
    program_sun->set_uniform_mat4("view", view_mat_sun);
    program_sun->set_uniform_mat4("projection", projection);
    program_sun->set_uniform_vec3("color_border", params.color_border.x, params.color_border.y, params.color_border.z);
    program_sun->set_uniform_vec3("color_center", params.color_center.x, params.color_center.y, params.color_center.z);
    //program_sun->set_uniform_vec3("color_inner_ring", params.color_inner_ring.x, params.color_inner_ring.y, params.color_inner_ring.z);
    program_sun->set_uniform_vec3("color_gradient", params.color_gradient.x, params.color_gradient.y, params.color_gradient.z);

    program_sun->set_uniform_int("sun_border", 0);
    program_sun->set_uniform_int("sun_eclat", 1);
    program_sun->set_uniform_int("sun_center", 2);
    program_sun->set_uniform_int("sun_inner_ring", 3);
    program_sun->set_uniform_int("sun_gradient", 4);

    glBindVertexArray(sunVAO);

    for (int i = 0; i < sun_textures.size(); ++i) {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, sun_textures[i]);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    //------------------ Clouds rendering --------------------------

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program_clouds->use();

    program_clouds->set_uniform_int("tex_cloud", 0);
    program_clouds->set_uniform_int("tex_cloud_mask", 1);
    view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
//    view = camera->view_matrix();
    if (params.offset > 1.0)
      params.offset = -1.0;
    program_clouds->set_uniform_float("offset", params.offset);

    glm::mat4 model_mat_clouds = glm::mat4(1.0f);
//    view = glm::translate(view, glm::vec3(-offset * 10));
    program_clouds->set_uniform_mat4("view", view);
    program_clouds->set_uniform_mat4("projection", projection);
    program_clouds->set_uniform_float("alpha_clip", params.alpha_clip);
    program_clouds->set_uniform_mat4("model", model_mat_clouds);
    program_clouds->set_uniform_vec3("camera_right", camera->right);
    program_clouds->set_uniform_vec3("camera_up", camera->up);
    params.offset += helper.deltaTime / 100;
//    std::cout <<


    glBindVertexArray(cloudsVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cloudsTextures[2]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudsTextures[3]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * 4);
    glBindVertexArray(0);


    if (params.display_depth_map) {

      quad_depth_shader->use();
      quad_depth_shader->set_uniform_int("depthMap", 0);
      quad_depth_shader->set_uniform_float("near_plane", params.near_plane_light);
      quad_depth_shader->set_uniform_float("far_plane", params.far_plane_light);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);
      renderQuad();
    }

    set_im_gui_options();

    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

}
}

