#include "CascadedShadow.h"

#include <cmath>
#include <iostream>
#include "misc.h"
#include "Model.h"
CascadedShadow::CascadedShadow(unsigned int nb_division, unsigned int shadow_width, unsigned int shadow_height)
  : nb_division(nb_division)
  , shadow_width(shadow_width)
  , shadow_height(shadow_height)
{
  depthMapTextures.resize(nb_division);
  lightSpaceMatrices.resize(nb_division);
  generate_depth_map_texture();
  generate_depth_map_frame_buffer();
  shadow_shader_depth = init_program("shaders/vertex_shadow_depth.glsl",
                                     "shaders/fragment_shadow_depth.glsl");
}

//Generate depth map texture
void CascadedShadow::generate_depth_map_texture() {
  glGenTextures(nb_division, depthMapTextures.data());

  for (unsigned int i = 0; i < nb_division; ++i) {
    glBindTexture(GL_TEXTURE_2D, depthMapTextures[i]);
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
}

void CascadedShadow::generate_depth_map_frame_buffer() {
  glGenFramebuffers(1, &depthMapFBO);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  // We select the first depthMapTexture for binding just to change the option of the frame buffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTextures[0], 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::vector<glm::mat4> CascadedShadow::computeShadowCascaded(const windfall::Param& params, std::shared_ptr<Camera> camera
     , Model& windfall_lowres, int SRC_WIDTH, int SRC_HEIGHT
    , const glm::mat4& view, const glm::vec3& lightDir, const glm::mat4& model_mat_windfall) {

  glm::vec3 eye = glm::vec3(params.light_pos[0], params.light_pos[1], params.light_pos[2]);

  glViewport(0, 0, this->shadow_width, this->shadow_height);

  glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
  this->shadow_shader_depth->set_uniform_mat4("model", model_mat_windfall);

  if (params.peter_paning) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
  }

  for (unsigned int i = 0; i < this->nb_division; ++i) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthMapTextures[i], 0); //Bind right depth texture to render to
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera->fov_camera), (float)SRC_WIDTH / (float)SRC_HEIGHT,
                                            this->cascades_delimitations[i], this->cascades_delimitations[i + 1]);
    this->lightSpaceMatrices[i] = computeLightViewProjMatrix(eye, lightDir, view, projection, params.coeff_increase_shadow_frustum_z, params.coeff_increase_shadow_frustum_xy);
    this->shadow_shader_depth->set_uniform_mat4("lightSpaceMatrix", this->lightSpaceMatrices[i]);
    this->shadow_shader_depth->set_uniform_float("alpha_clip", params.alpha_clip);

    windfall_lowres.draw(this->shadow_shader_depth);
  }


  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // reset viewport

  glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  return this->lightSpaceMatrices;
}


// This function will return the coordinates of the frustrum we are currently using to clip object

std::vector<glm::vec4> get_frustrum_world_space_coordinates(const glm::mat4& view, const glm::mat4& projection) {
  // The coordinates of the frustrum in the normalized device coordinate system always range from -1 to 1
  std::vector<glm::vec4> coords_ndc = {
      glm::vec4{-1,-1,-1, 1},
      glm::vec4{-1,-1,1, 1},
      glm::vec4{-1,1,-1, 1},
      glm::vec4{-1,1,1, 1},
      glm::vec4{1,-1,-1, 1},
      glm::vec4{1,-1,1, 1},
      glm::vec4{1,1,-1, 1},
      glm::vec4{1,1,1, 1},
  };

  glm::mat4 ndc_to_world = glm::inverse(projection * view);
  std::vector<glm::vec4> coords_world;
  for (const auto& coord : coords_ndc) {
    glm::vec4 coord_world = ndc_to_world * coord;
    coords_world.emplace_back(coord_world / coord_world.w); // Ensure that w is 1
  }
  /*
  std::cout << std::setprecision(2) << std::fixed;
    for (auto frustrum_coord : coords_world) {
      std::cout << frustrum_coord[0] << ' ' << frustrum_coord[1] << ' ' << frustrum_coord[2] << " | ";
    }
    std::cout << std::endl;
    */

  return coords_world;
}

glm::mat4 computeLightViewProjMatrix(const glm::vec3& lightPos, const glm::vec3& lightDir, const glm::mat4& view
                                     , const glm::mat4& projection, float increase_coeff_z, float increase_coeff_xy) {
  auto frustrum_coords = get_frustrum_world_space_coordinates(view, projection);

  glm::vec3 frustrum_center = glm::vec3(0, 0, 0);
  for (auto& coord_world : frustrum_coords) {
    frustrum_center += glm::vec3(coord_world);
  }
  frustrum_center /= frustrum_coords.size();

  glm::vec3 lookDir = frustrum_center - lightDir;
  glm::mat4 lightView = glm::lookAt(lookDir, frustrum_center, glm::vec3(0.f, 1.f, 0.f));

  for (auto& coord_world : frustrum_coords) {
    coord_world = lightView * coord_world;
    // Transform the world coordinates from the camera frustrum into light view space, so we can find the bounds of the new frustrum
  }
  float left = frustrum_coords[0].x;
  float right = frustrum_coords[0].x;
  float bottom = frustrum_coords[0].y;
  float top = frustrum_coords[0].y;
  float near = frustrum_coords[0].z;
  float far = frustrum_coords[0].z;


  for (const auto& coord : frustrum_coords) {
    left =   std::min(left,   coord.x);
    right =  std::max(right,  coord.x);
    bottom = std::min(bottom, coord.y);
    top =    std::max(top,    coord.y);
    near =   std::min(near,   coord.z);
    far =    std::max(far,    coord.z);
  }


  if (near < 0.f)
    near *= increase_coeff_z;
  else
    near /= increase_coeff_z;
  if (far < 0.f)
    far /= increase_coeff_z;
  else
    far *= increase_coeff_z;



  if (bottom < 0.f)
    bottom *= increase_coeff_xy;
  else
    bottom /= increase_coeff_xy;
  if (top < 0.f)
    top /= increase_coeff_xy;
  else
    top *= increase_coeff_xy;

  if (left < 0.f)
    left *= increase_coeff_xy;
  else
    left /= increase_coeff_xy;
  if (right < 0.f)
    right/= increase_coeff_xy;
  else
    right *= increase_coeff_xy;


  glm::mat4 lightProj = glm::ortho(left, right, bottom, top, near, far);

  return lightProj * lightView;
}