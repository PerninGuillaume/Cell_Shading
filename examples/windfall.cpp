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
#include "../CascadedShadow.h"
#include "../windfall_objects.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace windfall {

float lastFrame = 0.0f;
unsigned int NB_CASCADES = 2;

std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

struct {
  bool with_lighting = true;
  bool wireframe = false;
  bool use_zAtoon = false;
  bool no_texture = false;
  bool display_normals = false;
  bool use_shadow = true;
  bool use_cascaded_shadow = false;
  int cascadeLevel = 0;
  bool cascade_show_layers = false;
  bool fitLightFrustrum = true;
  bool display_depth_map = false;
  bool peter_paning = false;
  bool pcf = true;
  float shadow_bias = 0.005f;
  float near_plane_light = 20.0f, far_plane_light = 90.0f;
  float light_ambient = 0.7f;
  float light_diffuse = 0.8f;
  float light_dir[3] = {0.3f, -0.57f, -0.2f};
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
  ImVec4 color_gradient = ImVec4(146 / 255.0f, 145 / 255.0f, 7 / 255.0f, 1.0f);
  float sun_magnification = 330.0f;
  float lowest_eye_cancer = 0.1f;
  bool hd = false;
  float wave_height = -10.05f;
  float sea_height = -10.267f;
  float billboard_size[2] = {5.0f, 1.0f};
} params;

void set_im_gui_options(bool use_im_gui) {

  if (use_im_gui) {
    ImGui::Begin("Windfall options");
    ImGui::Checkbox("Hd textures", &params.hd);
    if (ImGui::TreeNode("Sun")) {
      ImGui::ColorPicker3("Center", (float*)&params.color_center);
      ImGui::ColorPicker3("Gradient", (float*)&params.color_gradient);
      ImGui::SliderFloat("Sun magnification", &params.sun_magnification, 0.0f, 1000.0f);
      ImGui::SliderFloat("Lowest Eye cancer", &params.lowest_eye_cancer, 0.0f, 1.0f);

      ImGui::TreePop();
      ImGui::Separator();
    }
    if (ImGui::TreeNode("Lighting")) {
      ImGui::Checkbox("Enable lighting", &params.with_lighting);
      ImGui::SliderFloat("Light diffuse", &params.light_diffuse, 0.0f, 1.0f);
      ImGui::SliderFloat("Light ambient", &params.light_ambient, 0.0f, 1.0f);
      ImGui::Checkbox("Use Zatoon", &params.use_zAtoon);

      ImGui::TreePop();
      ImGui::Separator();
    }

    if (ImGui::TreeNode("Shadow")) {
      ImGui::Checkbox("Shadow", &params.use_shadow);
      ImGui::Checkbox("Use cascaded shadow", &params.use_cascaded_shadow);
      ImGui::Checkbox("FitLightFrustrum", &params.fitLightFrustrum);
      ImGui::Checkbox("Depth texture", &params.display_depth_map);
      ImGui::Checkbox("Color Cascade Layer", &params.cascade_show_layers);
      ImGui::SliderInt("Cascade depth texture", &params.cascadeLevel, 0, NB_CASCADES - 1);
      ImGui::Checkbox("Peter Paning", &params.peter_paning);
      ImGui::Checkbox("PCF", &params.pcf);
      ImGui::SliderFloat("Shadow bias", &params.shadow_bias, 0.0f, 0.1f);
      ImGui::SliderFloat3("Light position", params.light_pos, -100.0f, 100.0f);
      ImGui::SliderFloat3("Light shadow center", params.light_shadow_center, -100.0f, 100.0f);
      ImGui::SliderFloat3("Light direction", params.light_dir, -1.0f, 1.0f);

        ImGui::TreePop();
        ImGui::Separator();
    }
      if (ImGui::TreeNode("Water")) {
          ImGui::SliderFloat("Billboard size x", &params.billboard_size[0], 0.0f, 100.0f);
          ImGui::SliderFloat("Billboard size y", &params.billboard_size[1], 0.0f, 10.0f);

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

std::vector<glm::mat4> computeShadow(const Shadow& shadow, Model& windfall_lowres, int SRC_WIDTH, int SRC_HEIGHT
                               , const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model_mat_windfall) {
  glm::mat4 lightProjection, lightView;
  glm::mat4 lightSpaceMatrix;
  lightProjection = glm::ortho(params.ortho_bounds[0], params.ortho_bounds[1], params.ortho_bounds[2], params.ortho_bounds[3], params.near_plane_light, params.far_plane_light);
  glm::vec3 eye = glm::vec3(params.light_pos[0], params.light_pos[1], params.light_pos[2]);
  glm::vec3 center = glm::vec3(params.light_shadow_center[0], params.light_shadow_center[1], params.light_shadow_center[2]);
  glm::vec3 up = glm::vec3(0.6f, 0.02f, -0.77f);
  lightView = glm::lookAt(eye,
                          center,
                          up);

  glm::vec3 lightDir = glm::vec3(params.light_dir[0], params.light_dir[1], params.light_dir[2]);
  if (params.fitLightFrustrum)
    lightSpaceMatrix= computeLightViewProjMatrix(eye, lightDir, view, projection);
  else
    lightSpaceMatrix = lightProjection * lightView;

  glViewport(0, 0, shadow.shadow_width, shadow.shadow_height);
  glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);



  shadow.shadow_shader_depth->set_uniform_mat4("model", model_mat_windfall);
  shadow.shadow_shader_depth->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);

  if (params.peter_paning) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
  }
  windfall_lowres.draw(shadow.shadow_shader_depth);
  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // reset viewport

  glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  return {lightSpaceMatrix};
}

std::vector<glm::mat4> computeShadowCascaded(CascadedShadow& shadow, Model& windfall_lowres, int SRC_WIDTH, int SRC_HEIGHT
    , const glm::mat4& view, const glm::mat4& model_mat_windfall) {

  glm::vec3 eye = glm::vec3(params.light_pos[0], params.light_pos[1], params.light_pos[2]);
  glm::vec3 lightDir = glm::vec3(params.light_dir[0], params.light_dir[1], params.light_dir[2]);

  glViewport(0, 0, shadow.shadow_width, shadow.shadow_height);

  glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
  shadow.shadow_shader_depth->set_uniform_mat4("model", model_mat_windfall);

  if (params.peter_paning) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
  }

  for (unsigned int i = 0; i < shadow.nb_division; ++i) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow.depthMapTextures[i], 0); //Bind right depth texture to render to
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera->fov_camera), (float)SRC_WIDTH / (float)SRC_HEIGHT,
                                  shadow.cascades_delimitations[i], shadow.cascades_delimitations[i + 1]);
    shadow.lightSpaceMatrices[i] = computeLightViewProjMatrix(eye, lightDir, view, projection);
    shadow.shadow_shader_depth->set_uniform_mat4("lightSpaceMatrix", shadow.lightSpaceMatrices[i]);

    windfall_lowres.draw(shadow.shadow_shader_depth);
  }


  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // reset viewport

  glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  return shadow.lightSpaceMatrices;
}

void display_water(program* program_water, GLuint waterVAO, const glm::mat4& view, const glm::mat4& projection
                   , const glm::mat4& lightSpaceMatrix, const Shadow& shadow) {
    program_water->use();
    glm::mat4 model_mat_water = glm::mat4(1.0f);
    program_water->set_uniform_mat4("view", view);
    program_water->set_uniform_mat4("projection", projection);
    program_water->set_uniform_mat4("model", model_mat_water);

    program_water->set_uniform_bool("use_shadow", params.use_shadow);
    program_water->set_uniform_bool("pcf", params.pcf);
    program_water->set_uniform_float("shadow_bias", params.shadow_bias);
    program_water->set_uniform_int("shadowMap", 0);
    program_water->set_uniform_mat4("lightSpaceMatrices_cascade", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);

    glBindVertexArray(waterVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void display_normals(program* shader_normals, const glm::mat4& projection, const glm::mat4& view,
                     const glm::mat4& model_mat_windfall, bool load_hd_texture, Model& windfall_highres, Model& windfall_lowres) {
    if (params.display_normals) {
        shader_normals->set_uniform_mat4("projection", projection);
        shader_normals->set_uniform_mat4("view", view);
        shader_normals->set_uniform_mat4("model", model_mat_windfall);

        if (params.hd && load_hd_texture)
            windfall_highres.draw(shader_normals);
        else
            windfall_lowres.draw(shader_normals);

    }
}

void display_skybox(program *program_skybox, GLuint skyboxVAO, GLuint cubemapTexture, glm::mat4 projection) {
    program_skybox->use();

    glDepthFunc(GL_LEQUAL);
    glm::mat4 view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
    program_skybox->set_uniform_mat4("view", view);
    program_skybox->set_uniform_mat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void display_shore(program* program_shore, const Helper& helper, GLuint shoreVAO, const glm::mat4& projection
        , const std::vector<unsigned int>& shoreTextures, const glm::mat4 lightSpaceMatrix, const Shadow& shadow
        , int nb_of_shore_waves) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program_shore->use();

    program_shore->set_uniform_int("tex_alpha_wave", 0);
    program_shore->set_uniform_int("tex_black_wave", 1);
    program_shore->set_uniform_int("tex_limit_wave", 2);
    program_shore->set_uniform_int("tex_mask_wave", 3);

    glm::mat4 view = camera->view_matrix();
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

    glBindVertexArray(shoreVAO);
    for (size_t i = 0; i < shoreTextures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, shoreTextures[i]);
    }

    program_shore->set_uniform_bool("use_shadow", params.use_shadow);
    program_shore->set_uniform_bool("pcf", params.pcf);
    program_shore->set_uniform_float("shadow_bias", params.shadow_bias);
    program_shore->set_uniform_int("shadowMap", 4);
    program_shore->set_uniform_mat4("lightSpaceMatrices_cascade", lightSpaceMatrix);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);

    glDrawArrays(GL_TRIANGLES, 0, 6 * nb_of_shore_waves);

}

void display_clouds(program* program_clouds, GLuint cloudsVAO, const Helper& helper
                    , const std::vector<unsigned int>& cloudsTextures, const glm::mat4& projection) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program_clouds->use();

    program_clouds->set_uniform_int("tex_cloud", 0);
    program_clouds->set_uniform_int("tex_cloud_mask", 1);
    glm::mat4 view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
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


    glBindVertexArray(cloudsVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cloudsTextures[2]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudsTextures[3]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * 4);
    glBindVertexArray(0);

}

void display_sun(program* program_sun, GLuint sunVAO, const std::vector<unsigned int>& sun_textures
                 , const glm::mat4& projection, float alignment, float alignment_limit) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glm::mat4 model_mat_sun = glm::mat4(1.0f);
    model_mat_sun = glm::scale(model_mat_sun, glm::vec3(1.0f));
    program_sun->set_uniform_mat4("model", model_mat_sun);
    glm::mat4 view_mat_sun = glm::mat4(glm::mat3(camera->view_matrix()));


    if (alignment > alignment_limit) {
        float translation = -(alignment - alignment_limit) * 10 * params.sun_magnification;
        view_mat_sun = glm::translate(view_mat_sun, glm::vec3(0.0f, translation, 0.0f));
    }
    //glm::mat4 view_mat_sun = camera->view_matrix();
    program_sun->set_uniform_mat4("view", view_mat_sun);
    program_sun->set_uniform_mat4("projection", projection);
    program_sun->set_uniform_vec3("color_border", params.color_border.x, params.color_border.y, params.color_border.z);
    program_sun->set_uniform_vec3("color_center", params.color_center.x, params.color_center.y, params.color_center.z);
    program_sun->set_uniform_vec3("color_gradient", params.color_gradient.x, params.color_gradient.y, params.color_gradient.z);

    program_sun->set_uniform_int("sun_border", 0);
    program_sun->set_uniform_int("sun_eclat", 1);
    program_sun->set_uniform_int("sun_center", 2);
    program_sun->set_uniform_int("sun_inner_ring", 3);
    program_sun->set_uniform_int("sun_gradient", 4);

    glBindVertexArray(sunVAO);

    for (size_t i = 0; i < sun_textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, sun_textures[i]);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

}

void display_waves(program* program_waves, const std::vector<unsigned int>& waves_VAO, const glm::mat4& projection
                    , const glm::vec3& center_of_waves, std::vector<bool>& wave_new_cycle_has_begun
                    , const std::vector<glm::vec3>& waves_center, GLuint wavesTexture) {
    program_waves->set_uniform_int("tex_wave", 0);
//    view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
    glm::mat4 view = camera->view_matrix();

    glm::mat4 model_mat_waves = glm::mat4(1.0f);
//    view = glm::translate(view, glm::vec3(-offset * 10));
    program_waves->set_uniform_mat4("view", view);
    program_waves->set_uniform_mat4("projection", projection);
    program_waves->set_uniform_float("alpha_clip", params.alpha_clip);
    program_waves->set_uniform_mat4("model", model_mat_waves);
    program_waves->set_uniform_vec3("camera_right", camera->right);
    program_waves->set_uniform_vec3("camera_up", camera->up);
    std::vector<glm::vec3> billboard_sizes(waves_VAO.size(), glm::vec3(params.billboard_size[0], params.billboard_size[1], 0.0f));
    //glm::vec3 billboard_size = glm::vec3(params.billboard_size[0], params.billboard_size[1], 0.0f);


    program_waves->set_uniform_vec3("center_waves", center_of_waves);
    for (size_t i = 0; i < waves_VAO.size(); ++i) {
        float percentage_apex = glm::sin(glfwGetTime() + (float)i);
        //std::cout << i << ":" << percentage_apex << std::endl;
        //float percentage_apex = (glm::sin(glfwGetTime()) + 1.0f) / 2.0f;
        float percentage_displacement = fmod(glfwGetTime() + (float)i, 2 * PI) / (2 * PI);
        //std::cout << percentage_apex << " " << percentage_displacement << std::endl;
        if (percentage_apex < 0.0f && !wave_new_cycle_has_begun[i]) {
            wave_new_cycle_has_begun[i] = true;
            billboard_sizes[i][0] += rand() % 100;
            billboard_sizes[i][1] += rand() % 100;
        } else if (percentage_apex > 0.0f) {
            wave_new_cycle_has_begun[i] = false;
        }
        program_waves->set_uniform_float("apex_percentage", percentage_apex);
        program_waves->set_uniform_float("dispacement_percentage", percentage_displacement);
        program_waves->set_uniform_vec3("billboard_size", billboard_sizes[i][0],
                                        billboard_sizes[i][1] * percentage_apex, 0.0f);
        program_waves->set_uniform_vec3("wave_center", waves_center[i]);
        //display_glm_vec3(waves_center[i]);
        glBindVertexArray(waves_VAO[i]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wavesTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
}

void display_wind(program* program_wind, GLuint windVAO, const glm::mat4& view, const glm::mat4& projection) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  static const size_t nb_blow_of_wind = 20;
  static std::array<glm::vec3, nb_blow_of_wind> translations{};
  static std::array<glm::vec3, nb_blow_of_wind> directions{};
  static std::array<float, nb_blow_of_wind> time_initialized{};
  static std::array<float, nb_blow_of_wind> time_before_next_animation{};
  static std::array<GLint, nb_blow_of_wind> in_animation{};
  static std::array<float, nb_blow_of_wind> animation_percentage{};
  static std::array<float, nb_blow_of_wind> phase_vector{};

  float max_rand_value = 2.f * 3.14f;
  float total_animation_time = 3.f;
  double time = glfwGetTime();
  for (size_t i = 0; i < nb_blow_of_wind; ++i) {
    float time_elapsed = time - time_initialized[i];
    animation_percentage[i] = time_elapsed / total_animation_time;
    if (time_elapsed < total_animation_time)
      continue; // Not yet the time to change the position of the animation

    in_animation[i] = 0;

    if (time < time_before_next_animation[i])
      continue;

    in_animation[i] = 1;
    time_initialized[i] = time;
    time_before_next_animation[i] = time + (float)rand()/(float)(RAND_MAX) * 10.f;

    int radius_max = 100;
    int radius_min = 60;
    float radius = (rand()%(radius_max-radius_min + 1) + radius_min);
    float rand_angle = ((float)rand()/(float)(RAND_MAX)) * max_rand_value;
    float x = glm::cos(rand_angle) * radius;
    float z = glm::sin(rand_angle) * radius;
    translations[i] = glm::vec3(-10.f + x, 5.f, -30.f + z);

    float max_rand_dir_value = 0.3f;
    float rand_dir_angle = ((float)rand()/(float)(RAND_MAX)) * max_rand_dir_value - max_rand_dir_value / 2.f;
    directions[i] = glm::vec3(1.0f + rand_dir_angle, rand_dir_angle, rand_dir_angle);

    int period_min = 1;
    int period_max = 10;
    phase_vector[i] = (rand()%(period_max-period_min + 1) + period_min);
  }

  program_wind->use();


  program_wind->set_uniform_vector_vec3("offsets", translations.size(), translations.data());
  program_wind->set_uniform_vector_vec3("directions", directions.size(), directions.data());
  program_wind->set_uniform_vector_bool("in_animation", in_animation.size(), in_animation.data());
  program_wind->set_uniform_vector_float("animation_percentage", animation_percentage.size(), animation_percentage.data());
  program_wind->set_uniform_vector_float("phase_vector", phase_vector.size(), phase_vector.data());
  glm::mat4 model_mat_wind = glm::mat4(1.0f);
  program_wind->set_uniform_mat4("view", view);
  program_wind->set_uniform_mat4("projection", projection);
  program_wind->set_uniform_mat4("model", model_mat_wind);
  program_wind->set_uniform_vec3("camera_front", camera->front);
  glBindVertexArray(windVAO);
  glDrawArraysInstanced(GL_POINTS, 0, 1, nb_blow_of_wind);

}

void display(GLFWwindow *window, bool load_hd_texture, bool use_im_gui) {
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

  //--------------------------------Shader setup--------------------------------------
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
  program *program_waves = init_program("shaders/vertex_wave.glsl",
                                      "shaders/fragment_wave.glsl");
  program *program_wind = init_program("shaders/vertex_wind.glsl"
      , "shaders/fragment_wind.glsl", "shaders/geometry_wind.glsl");

  //-------------------------------Model and texture loading------------------------------
  Model windfall_highres;
  if (load_hd_texture)
    windfall_highres = Model("models/textures_windfall_/textures_windfall_highres/Windfall.obj");
  Model windfall_lowres("models/textures_windfall_/textures_windfall_lowres/Windfall.obj");


  unsigned int size_shadow_texture = 4096;
  Shadow shadow = Shadow(size_shadow_texture, size_shadow_texture);
  CascadedShadow cascaded_shadow = CascadedShadow(NB_CASCADES, size_shadow_texture, size_shadow_texture);

  glEnable(GL_DEPTH_TEST);

  // Skybox
  unsigned int cubemapTexture = loadSkyBox(program_skybox);
  unsigned int skyboxVAO = skyBox_create_VAO();

  // Water
  unsigned int waterVAO = water_create_VAO(params.sea_height);
  std::vector<unsigned int> shoreTextures = loadShore();
  int nb_of_shore_waves;
  unsigned int shoreVAO = shore_create_VAO(nb_of_shore_waves, params.wave_height);

  // Zatoon
  set_zAtoon(program_windfall);

  // Sun
  std::vector<unsigned int> sun_textures = loadSunTextures();
  unsigned int sunVAO = sun_create_VAO();

  // Clouds
  std::vector<unsigned int> cloudsTextures = loadClouds();
  unsigned int cloudsVAO = clouds_create_VAO();

  // Waves
  unsigned int wavesTexture = loadWaves();
  std::vector<glm::vec3> waves_center;
  glm::vec3 center_of_waves = glm::vec3(-3.0f, 0.0f, -20.0f);
  std::vector<unsigned int> waves_VAO = waves_create_VAO_vector(waves_center, center_of_waves);
  std::vector<bool> wave_new_cycle_has_begun(waves_VAO.size(), false);

  // Wind
  unsigned int windVAO = wind_create_VAO();

  Helper helper = Helper(camera, use_im_gui);

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
    projection = glm::perspective(glm::radians(camera->fov_camera), (float)SRC_WIDTH / (float)SRC_HEIGHT,
                                  0.1f, 300.0f);

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

    glm::mat4 model_mat_windfall = glm::mat4(1.0f);
    model_mat_windfall = glm::translate(model_mat_windfall, glm::vec3(1.0f, -10.0f, -25.0f));
    model_mat_windfall = glm::scale(model_mat_windfall, glm::vec3(0.01f, 0.01f, 0.01f));

    // 1. Render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------

    std::vector<glm::mat4> lightSpaceMatrices;
    lightSpaceMatrices = computeShadowCascaded(cascaded_shadow, windfall_lowres, SRC_WIDTH, SRC_HEIGHT, view, model_mat_windfall);
    //lightSpaceMatrices = computeShadow(shadow, windfall_lowres, SRC_WIDTH, SRC_HEIGHT
    //    , view, projection, model_mat_windfall);

      // 2. render scene as normal using the generated depth/shadow map
    // --------------------------------------------------------------


    float alignment = glm::dot(camera->front, glm::vec3(0.0f, 1.0f, 0.0f));
    // Variable measuring by how much we are looking at the sun, 1.0f being the highest possible value
    float alignment_limit = 0.9f; // By this value we can view the sun
    float eye_cancer = get_eye_cancer(alignment, alignment_limit, params.lowest_eye_cancer);

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

    program_windfall->set_uniform_vec3("dirLight.ambient", params.light_ambient * eye_cancer);
    program_windfall->set_uniform_vec3("dirLight.diffuse", params.light_diffuse * eye_cancer);

    std::vector<int> shadowMap_sampler_index;
    for (unsigned int i = 0; i < NB_CASCADES; ++i) {
      shadowMap_sampler_index.emplace_back(i + 1);
    }
    program_windfall->set_uniform_bool("color_cascade_layer", params.cascade_show_layers);
    program_windfall->set_uniform_vector_int("shadowMap_cascade", shadowMap_sampler_index.size(), shadowMap_sampler_index.data());
    //program_windfall->set_uniform_int("shadowMap", 1);
    //program_windfall->set_uniform_bool("useCascadedShadow", params.use_cascaded_shadow);
    program_windfall->set_uniform_vector_float("cascade_z_limits", cascaded_shadow.cascades_delimitations);
    //program_windfall->set_uniform_vector_mat4("lightSpaceMatrices_cascade", lightSpaceMatrices);
    program_windfall->set_uniform_mat4("lightSpaceMatrices_cascade0", lightSpaceMatrices[0]);
    program_windfall->set_uniform_mat4("lightSpaceMatrices_cascade1", lightSpaceMatrices[1]);
    //program_windfall->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrices[0]);
    for (unsigned int i = 0; i < NB_CASCADES; ++i) {
      glActiveTexture(GL_TEXTURE1 + i);
      glBindTexture(GL_TEXTURE_2D, cascaded_shadow.depthMapTextures[i]);
    }


    if (params.hd && load_hd_texture)
      windfall_highres.draw(program_windfall);
    else
      windfall_lowres.draw(program_windfall);


    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //display_water(program_water, waterVAO, view, projection, lightSpaceMatrix, shadow);

    display_normals(shader_normals, projection, view, model_mat_windfall, load_hd_texture, windfall_highres, windfall_lowres);

    display_skybox(program_skybox, skyboxVAO, cubemapTexture, projection);

    //display_shore(program_shore, helper, shoreVAO, projection, shoreTextures, lightSpaceMatrix, shadow, nb_of_shore_waves);

    display_sun(program_sun, sunVAO, sun_textures, projection, alignment, alignment_limit);

    display_clouds(program_clouds, cloudsVAO, helper, cloudsTextures, projection);

    display_waves(program_waves, waves_VAO, projection, center_of_waves, wave_new_cycle_has_begun, waves_center, wavesTexture);

    display_wind(program_wind, windVAO, view, projection);

      //------------------Depth Map----------------------------------
    if (params.display_depth_map) {

      quad_depth_shader->use();
      quad_depth_shader->set_uniform_int("depthMap", 0);
      quad_depth_shader->set_uniform_float("near_plane", params.near_plane_light);
      quad_depth_shader->set_uniform_float("far_plane", params.far_plane_light);
      glActiveTexture(GL_TEXTURE0);
      //glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);
      glBindTexture(GL_TEXTURE_2D, cascaded_shadow.depthMapTextures[params.cascadeLevel]);
      renderQuad();
    }

    set_im_gui_options(use_im_gui);

    glfwGetWindowSize(window, &SRC_WIDTH, &SRC_HEIGHT);
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

