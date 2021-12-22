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
unsigned int NB_CASCADES = 3;
const unsigned int NB_WAVES = 1000;

std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
Param params;

void set_im_gui_options(bool use_im_gui) {

  if (use_im_gui) {
    ImGui::Begin("Windfall options");
    ImGui::Checkbox("Hd textures", &params.hd);
    if (ImGui::TreeNode("Sun")) {
      ImGui::Checkbox("Moving Sun", &params.cycle_day);
      ImGui::ColorPicker3("Center", (float*)&params.color_center);
      ImGui::ColorPicker3("Gradient", (float*)&params.color_gradient);
      ImGui::ColorPicker3("Additional Color Skybox", (float*)&params.color_skybox_change);
      ImGui::SliderFloat("Blend Percentage Skybox", &params.blend_skybox_color, 0.0f, 1.0f);
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
      if (ImGui::Checkbox("Use cascaded shadow", &params.use_cascaded_shadow)) {
        std::vector<std::string> fragment_filenames {"shaders/fragment_link.glsl", "shaders/fragment_shadow.glsl"};
        if (params.use_cascaded_shadow)
          params.program_windfall_with_lighting = init_program("shaders/vertex_link.glsl",
                                                               fragment_filenames, "",
                                                               {{"NB_CASCADES_TO_REPLACE", std::to_string(NB_CASCADES)}});
        else
          params.program_windfall_with_lighting = init_program("shaders/vertex_link.glsl",
                                                               fragment_filenames, "",
                                                               {{"NB_CASCADES_TO_REPLACE", "1"}});
      }
      ImGui::Checkbox("blend_between_cascade", &params.blend_between_cascade);
      ImGui::Checkbox("FitLightFrustrum", &params.fitLightFrustrum);
      ImGui::Checkbox("Depth texture", &params.display_depth_map);
      ImGui::Checkbox("Color Cascade Layer", &params.cascade_show_layers);
      ImGui::SliderInt("Cascade depth texture", &params.cascadeLevel, 0, NB_CASCADES - 1);
      ImGui::Checkbox("Peter Paning", &params.peter_paning);
      ImGui::Checkbox("PCF", &params.pcf);
      ImGui::SliderInt("nb_samples_pcf", &params.square_sample_size, 0, 15);
      ImGui::SliderFloat("Shadow bias", &params.shadow_bias, 0.0001f, 0.2f);
      ImGui::SliderFloat3("Shadow biases", params.shadow_biases.data(), 0.001f, 0.4f);
      ImGui::SliderFloat("increase_shadow_frustum_z", &params.coeff_increase_shadow_frustum_z, 1.f, 5.f);
      ImGui::SliderFloat("increase_shadow_frustum_xy", &params.coeff_increase_shadow_frustum_xy, 1.f, 5.f);
      ImGui::SliderFloat("Angle sun", &params.angle_sun, 0.f, 360.f);
      ImGui::SliderFloat3("Light position", params.light_pos, -100.0f, 100.0f);
      ImGui::SliderFloat3("Light shadow center", params.light_shadow_center, -100.0f, 100.0f);

        ImGui::TreePop();
        ImGui::Separator();
    }
      if (ImGui::TreeNode("Water")) {
          ImGui::SliderFloat("Billboard size x", &params.billboard_size[0], 0.0f, 100.0f);
          ImGui::SliderFloat("Billboard size y", &params.billboard_size[1], 0.0f, 10.0f);
          ImGui::SliderFloat("Water Height", &params.sea_height, -9.5f, -10.5f);

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

void set_shadow_uniforms(program* program, const CascadedShadow& cascaded_shadow,
                         const Shadow& shadow, const std::vector<glm::mat4>& lightSpaceMatrices, int texture_offset_index) {
  std::vector<int> shadowMap_sampler_index;
  if (params.use_cascaded_shadow) {
    for (unsigned int i = 0; i < NB_CASCADES; ++i)
      shadowMap_sampler_index.emplace_back(i + texture_offset_index);
  } else {
    shadowMap_sampler_index.emplace_back(texture_offset_index);
  }

  program->set_uniform_bool("use_shadow", params.use_shadow);
  program->set_uniform_float("shadow_bias", params.shadow_bias);
  program->set_uniform_int("square_sample_size", params.square_sample_size);
  program->set_uniform_bool("pcf", params.pcf);
  program->set_uniform_bool("color_cascade_layer", params.cascade_show_layers);
  program->set_uniform_int("first_shadowMap_cascade", shadowMap_sampler_index[0]);
  program->set_uniform_int("second_shadowMap_cascade", shadowMap_sampler_index[1]);
  program->set_uniform_int("third_shadowMap_cascade", shadowMap_sampler_index[2]);
  program->set_uniform_bool("blend_between_cascade", params.blend_between_cascade);

  if (params.use_cascaded_shadow) {

    program->set_uniform_vector_float("shadow_biases", params.shadow_biases.size(), params.shadow_biases.data());
    program->set_uniform_vector_float("cascade_z_limits", cascaded_shadow.cascades_delimitations);
    for (unsigned int i = 0; i < NB_CASCADES; ++i) {
      glActiveTexture(GL_TEXTURE0 + i + texture_offset_index);
      glBindTexture(GL_TEXTURE_2D, cascaded_shadow.depthMapTextures[i]);
    }
  }
  else {
    program->set_uniform_vector_float("shadow_biases", 1, &params.shadow_bias);
    program->set_uniform_vector_float("cascade_z_limits", {params.znear, params.zfar});
    glActiveTexture(GL_TEXTURE0 + texture_offset_index);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);
  }
  program->set_uniform_vector_mat4("lightSpaceMatrices_cascade", lightSpaceMatrices);

}

void display_depth_map(program* quad_depth_shader, const Shadow& shadow, const CascadedShadow& cascaded_shadow) {
  if (params.display_depth_map) {

    quad_depth_shader->use();
    quad_depth_shader->set_uniform_int("depthMap", 0);
    quad_depth_shader->set_uniform_float("near_plane", params.near_plane_light);
    quad_depth_shader->set_uniform_float("far_plane", params.far_plane_light);
    glActiveTexture(GL_TEXTURE0);
    if (params.use_cascaded_shadow)
      glBindTexture(GL_TEXTURE_2D, cascaded_shadow.depthMapTextures[params.cascadeLevel]);
    else
      glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);
    renderQuad_corner();
  }

}

void display_windfall(program* program_windfall, const glm::mat4& model_mat_windfall, const glm::mat4& view
  , const glm::mat4& projection, const glm::vec3& lightDir, float eye_cancer, const Shadow& shadow, const CascadedShadow& cascaded_shadow
  , const std::vector<glm::mat4>& lightSpaceMatrices, bool load_hd_texture, Model& windfall_highres, Model& windfall_lowres) {

  program_windfall->set_uniform_mat4("model", model_mat_windfall);
  program_windfall->set_uniform_mat4("view", view);
  program_windfall->set_uniform_mat4("projection", projection);
  program_windfall->set_uniform_float("alpha_clip", params.alpha_clip);
  program_windfall->set_uniform_bool("use_zAtoon", params.use_zAtoon);
  program_windfall->set_uniform_bool("no_texture", params.no_texture);

  program_windfall->set_uniform_vec3("dirLight.direction", lightDir);

  program_windfall->set_uniform_vec3("dirLight.ambient", params.light_ambient * eye_cancer);
  program_windfall->set_uniform_vec3("dirLight.diffuse", params.light_diffuse * eye_cancer);

  set_shadow_uniforms(program_windfall, cascaded_shadow, shadow, lightSpaceMatrices, 1);

  if (params.hd && load_hd_texture)
    windfall_highres.draw(program_windfall);
  else
    windfall_lowres.draw(program_windfall);

}

float compute_new_light_pos() {
  float radius = 50.f;
  float period_day = 380.0f;
  float t = fmod(glfwGetTime(), period_day);

  float angle_radian;
  if (params.cycle_day)
    angle_radian = 2 * M_PI * t / period_day;
  else
    angle_radian = params.angle_sun * M_PI / 180;
  float x = glm::cos(angle_radian) * radius;
  float y = glm::sin(angle_radian) * radius;

  // Change color of the sun based on angle
  float alpha = glm::sin(angle_radian);
  glm::vec4 color_center_zenith = glm::vec4(246 / 255.0f, 197 / 255.0f, 193 / 255.0f, 1.0f);
  glm::vec4 color_center_rising_sun = glm::vec4(255 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f);
  glm::vec4 color_angle = alpha * color_center_zenith + (1.f - alpha) * color_center_rising_sun;
  params.color_center = ImVec4(color_angle.r, color_angle.g, color_angle.b, color_angle.a);

  glm::vec3 color_skybox_rising_sun = glm::vec3(226 / 255.0f, 52 / 255.0f, 52 / 255.0f);
  glm::vec3 color_skybox_night = glm::vec3(0.f);
  float blend_percentage_rising_sun = 0.46f;
  float blend_percentage_night = 0.75f;

  glm::vec3 col_sky_additional;

  // On rising of the sun we want more of color_skybox_rising_sun, at midnight more of color_sybox_night
  // and at the zenith neither
  float percentage_rising = 1.f - std::abs(sin(angle_radian));
  float percentage_night = std::max(-1.f * sin(angle_radian), 0.);
  col_sky_additional = color_skybox_rising_sun * percentage_rising + color_skybox_night * percentage_night;
  params.blend_skybox_color = blend_percentage_rising_sun * percentage_rising + blend_percentage_night * percentage_night;
  params.color_skybox_change = ImVec4(col_sky_additional.r, col_sky_additional.g, col_sky_additional.b, 1.0f);

  // Store new position of light and return angle for computing the new vertex of the sun
  params.light_pos[0] = x;
  params.light_pos[1] = y;
  params.light_pos[2] = 0.f;
  params.light_shadow_center[0] = 0.f;
  params.light_shadow_center[1] = 0.f;
  params.light_shadow_center[2] = 0.f;

  return angle_radian;
}

void display_water(program* program_water, GLuint waterVAO, const glm::mat4& view, const glm::mat4& projection
    , const Shadow& shadow, const CascadedShadow& cascaded_shadow,
                   const std::vector<glm::mat4>& lightSpaceMatrices, const glm::vec3& lightDir) {
  program_water->use();
  glm::mat4 model_mat_water = glm::mat4(1.0f);
  program_water->set_uniform_mat4("view", view);
  program_water->set_uniform_mat4("projection", projection);
  program_water->set_uniform_mat4("model", model_mat_water);

  program_water->set_uniform_vec3("dirLight.direction", lightDir);
  program_water->set_uniform_vec3("dirLight.ambient", params.light_ambient);
  program_water->set_uniform_vec3("dirLight.diffuse", params.light_diffuse);


  set_shadow_uniforms(program_water, cascaded_shadow, shadow, lightSpaceMatrices, 0);

  glBindVertexArray(waterVAO);
  glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glEnable(GL_DEPTH_TEST);
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

void display_skybox(program *program_skybox, GLuint skyboxVAO, GLuint cubemapTexture, const glm::mat4& projection) {
  program_skybox->use();

  glDepthFunc(GL_LEQUAL);
  glm::mat4 view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
  program_skybox->set_uniform_mat4("view", view);
  program_skybox->set_uniform_mat4("projection", projection);
  program_skybox->set_uniform_vec3("additional_color", params.color_skybox_change.x, params.color_skybox_change.y, params.color_skybox_change.z);
  program_skybox->set_uniform_float("blend_percentage", params.blend_skybox_color);
  glBindVertexArray(skyboxVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
  glDepthFunc(GL_LESS);
}

void display_shore(program* program_shore, const Helper& helper, GLuint shoreVAO, const glm::mat4& projection
        , const std::vector<unsigned int>& shoreTextures, const Shadow& shadow, const CascadedShadow& cascaded_shadow
        , int nb_of_shore_waves, const std::vector<glm::mat4>& lightSpaceMatrices, const glm::vec3& lightDir) {
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

  program_shore->set_uniform_vec3("dirLight.direction", lightDir);
  program_shore->set_uniform_vec3("dirLight.ambient", params.light_ambient);
  program_shore->set_uniform_vec3("dirLight.diffuse", params.light_diffuse);
  set_shadow_uniforms(program_shore, cascaded_shadow, shadow, lightSpaceMatrices, 4);

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

  glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  glEnable(GL_DEPTH_TEST);
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
        program_waves->set_uniform_float("displacement_percentage", percentage_displacement);
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

void display_waves_instanced(program* program_waves, GLuint wave_VAO, const std::vector<GLuint> wave_VBOs
    , const glm::mat4& projection, const glm::vec3& center_of_waves, std::vector<bool>& wave_new_cycle_has_begun
    , std::vector<glm::vec3>& waves_center, GLuint wavesTexture) {

  std::vector<glm::vec3> billboard_sizes(NB_WAVES, glm::vec3(params.billboard_size[0], params.billboard_size[1], 0.0f));
  static std::array<float, NB_WAVES> percentage_displacements{};
  static std::array<float, NB_WAVES> apex_percentages{};

  program_waves->set_uniform_int("tex_wave", 0);
  glm::mat4 view = camera->view_matrix();

  program_waves->set_uniform_mat4("view", view);
  program_waves->set_uniform_mat4("projection", projection);
  program_waves->set_uniform_vec3("camera_right", camera->right);
  program_waves->set_uniform_vec3("camera_up", camera->up);


  program_waves->set_uniform_vec3("center_waves", center_of_waves);
  for (size_t i = 0; i < NB_WAVES; ++i) {
    float percentage_apex = glm::sin(glfwGetTime() + (float)i);
    float percentage_displacement = fmod(glfwGetTime() + (float)i, 2 * PI) / (2 * PI);
    if (percentage_apex < 0.0f && !wave_new_cycle_has_begun[i]) {
      wave_new_cycle_has_begun[i] = true;
      billboard_sizes[i][0] += rand() % 100;
      billboard_sizes[i][1] += rand() % 100;
    } else if (percentage_apex > 0.0f) {
      wave_new_cycle_has_begun[i] = false;
    }
    billboard_sizes[i] = glm::vec3(billboard_sizes[i][0],
                                    billboard_sizes[i][1] * percentage_apex, 0.0f);
    apex_percentages[i] = percentage_apex;
    percentage_displacements[i] = percentage_displacement;
  }

  glBindVertexArray(wave_VAO);

  glBindBuffer(GL_ARRAY_BUFFER, wave_VBOs[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * NB_WAVES, apex_percentages.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, wave_VBOs[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NB_WAVES, waves_center.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, wave_VBOs[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NB_WAVES, billboard_sizes.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, wave_VBOs[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * NB_WAVES, percentage_displacements.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, wavesTexture);
  glBindVertexArray(wave_VAO);
  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, NB_WAVES);
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

  //--------------------------------Shaders setup--------------------------------------
  program *quad_depth_shader = init_program("shaders/vertex_normalized_coord.glsl", "shaders/fragment_quad_depth.glsl");
  program *shader_normals = init_program("shaders/vertex_normals.glsl", "shaders/fragment_normals.glsl",
                                         "shaders/geometry_normals.glsl");
  program *program_windfall;
  program *program_windfall_without_lighting = init_program("shaders/vertex_model.glsl",
                                  "shaders/fragment_model.glsl");
  std::vector<std::string> fragment_filenames {"shaders/fragment_link.glsl", "shaders/fragment_shadow.glsl"};
  std::map<std::string, std::string> cascade_map_replace = {{"NB_CASCADES_TO_REPLACE", std::to_string(NB_CASCADES)}};
  program *program_windfall_with_lighting = init_program("shaders/vertex_link.glsl",
                                                         fragment_filenames, "", cascade_map_replace);

  program_windfall = program_windfall_with_lighting;
  program *program_skybox = init_program("shaders/vertex_skybox.glsl",
                                         "shaders/fragment_skybox.glsl");
  fragment_filenames = {"shaders/fragment_water.glsl", "shaders/fragment_shadow.glsl"};
  program *program_water = init_program("shaders/vertex_water.glsl",
                                        fragment_filenames, "", cascade_map_replace);
  program *program_clouds = init_program("shaders/vertex_clouds.glsl",
                                         "shaders/fragment_clouds.glsl");
  fragment_filenames = {"shaders/fragment_shore.glsl", "shaders/fragment_shadow.glsl"};
  program *program_shore = init_program("shaders/vertex_shore.glsl",
                                        fragment_filenames, "", cascade_map_replace);
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


  // Shadow
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

  // Clouds
  std::vector<unsigned int> cloudsTextures = loadClouds();
  unsigned int cloudsVAO = clouds_create_VAO();

  // Waves
  unsigned int wavesTexture = loadWaves();
  std::vector<glm::vec3> waves_center;
  glm::vec3 center_of_waves = glm::vec3(-3.0f, 0.0f, -20.0f);
  unsigned int waveVAO;
  std::vector<unsigned int> waveVBOs;
  std::tie(waveVAO, waveVBOs) = waves_create_VAO_vector(NB_WAVES, waves_center, center_of_waves);

  std::vector<bool> wave_new_cycle_has_begun(NB_WAVES, false);

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
                                  params.znear, params.zfar);

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

      glm::vec3 lightDir = glm::vec3(params.light_shadow_center[0] - params.light_pos[0],
                                     params.light_shadow_center[1] - params.light_pos[1],
                                     params.light_shadow_center[2] - params.light_pos[2]);

      std::vector<glm::mat4> lightSpaceMatrices;
      if (params.use_shadow) {
        lightDir = glm::normalize(lightDir);
        if (params.use_cascaded_shadow)
            lightSpaceMatrices = cascaded_shadow.computeShadowCascaded(params, camera, windfall_lowres, SRC_WIDTH,
                                                                       SRC_HEIGHT, view, lightDir, model_mat_windfall,
                                                                       waterVAO);
        else
            lightSpaceMatrices = shadow.computeShadow(params, windfall_lowres, SRC_WIDTH, SRC_HEIGHT, view, projection,
                                                      lightDir, model_mat_windfall, waterVAO);
    }

    // Display Objects for which you don't care about depth
    display_skybox(program_skybox, skyboxVAO, cubemapTexture, projection);

    float alignment = glm::dot(camera->front, glm::vec3(0.0f, 1.0f, 0.0f));
    // Variable measuring by how much we are looking at the sun, 1.0f being the highest possible value
    float alignment_limit = 0.9f; // By this value we can view the sun
    float eye_cancer = get_eye_cancer(alignment, alignment_limit, params.lowest_eye_cancer);
    float angle_sun = compute_new_light_pos();
    unsigned int sunVAO = sun_create_VAO(angle_sun);
    display_sun(program_sun, sunVAO, sun_textures, projection, alignment, alignment_limit);
    display_water(program_water, waterVAO, view, projection, shadow, cascaded_shadow, lightSpaceMatrices
        , lightDir);

      // 2. render scene as normal using the generated depth/shadow map
    // --------------------------------------------------------------

    display_windfall(program_windfall, model_mat_windfall, view, projection, lightDir, eye_cancer, shadow, cascaded_shadow
        , lightSpaceMatrices, load_hd_texture, windfall_highres, windfall_lowres);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    display_normals(shader_normals, projection, view, model_mat_windfall, load_hd_texture, windfall_highres, windfall_lowres);

    display_shore(program_shore, helper, shoreVAO, projection, shoreTextures, shadow, cascaded_shadow, nb_of_shore_waves
                  , lightSpaceMatrices, lightDir);


    display_clouds(program_clouds, cloudsVAO, helper, cloudsTextures, projection);

    display_waves_instanced(program_waves, waveVAO, waveVBOs, projection, center_of_waves, wave_new_cycle_has_begun, waves_center, wavesTexture);

    display_wind(program_wind, windVAO, view, projection);

    display_depth_map(quad_depth_shader, shadow, cascaded_shadow);

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
