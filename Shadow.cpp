#include <iomanip>
#include <iostream>

#include "Shadow.h"
#include "misc.h"
#include "CascadedShadow.h"

Shadow::Shadow(unsigned int shadow_width, unsigned int shadow_height)
    : shadow_width(shadow_width)
    , shadow_height(shadow_height)
{
  generate_depth_map_texture();
  generate_depth_map_frame_buffer();

  shadow_shader_depth = init_program("shaders/vertex_shadow_depth.glsl",
                                              "shaders/fragment_shadow_depth.glsl");
}
//Generate depth map texture
void Shadow::generate_depth_map_texture() {
  glGenTextures(1, &depthMapTexture);
  glBindTexture(GL_TEXTURE_2D, depthMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT,
               GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  //If we are out of the frustrum we are not in the shadow
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void Shadow::generate_depth_map_frame_buffer() {
  glGenFramebuffers(1, &depthMapFBO);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

std::vector<glm::mat4> Shadow::computeShadow(const windfall::Param& params, Model& windfall_lowres, int SRC_WIDTH, int SRC_HEIGHT
    , const glm::mat4& view, const glm::mat4& projection, const glm::vec3& lightDir, const glm::mat4& model_mat_windfall, GLuint waterVAO) {

  glm::mat4 lightProjection, lightView;
  glm::mat4 lightSpaceMatrix;
  lightProjection = glm::ortho(params.ortho_bounds[0], params.ortho_bounds[1], params.ortho_bounds[2], params.ortho_bounds[3], params.near_plane_light, params.far_plane_light);
  glm::vec3 eye = glm::vec3(params.light_pos[0], params.light_pos[1], params.light_pos[2]);
  glm::vec3 center = glm::vec3(params.light_shadow_center[0], params.light_shadow_center[1], params.light_shadow_center[2]);
  glm::vec3 up = glm::vec3(0.6f, 0.02f, -0.77f);
  lightView = glm::lookAt(eye,
                          center,
                          up);

  if (params.fitLightFrustrum)
    lightSpaceMatrix= computeLightViewProjMatrix(lightDir, view, projection, params.coeff_increase_shadow_frustum_z, params.coeff_increase_shadow_frustum_xy);
  else
    lightSpaceMatrix = lightProjection * lightView;

  glViewport(0, 0, this->shadow_width, this->shadow_height);
  glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);



  this->shadow_shader_depth->set_uniform_mat4("model", model_mat_windfall);
  this->shadow_shader_depth->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);
  this->shadow_shader_depth->set_uniform_float("alpha_clip", params.alpha_clip);

  if (params.peter_paning) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
  }
  windfall_lowres.draw(this->shadow_shader_depth);

  glBindVertexArray(waterVAO);
  //glDisable(GL_DEPTH_TEST);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  //glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);

  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // reset viewport

  glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  return {lightSpaceMatrix};

}
