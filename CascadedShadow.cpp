#include "CascadedShadow.h"

#include <cmath>
#include <iostream>
#include "misc.h"
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
                                     , const glm::mat4& projection, unsigned int size_texture, int i) {
  auto frustrum_coords = get_frustrum_world_space_coordinates(view, projection);

  glm::vec3 center = glm::vec3(0, 0, 0);
  for (auto& coord_world : frustrum_coords) {
    center += glm::vec3(coord_world);
  }
  center /= frustrum_coords.size();
  glm::mat4 lightView = glm::lookAt(center - lightDir, center, glm::vec3(0.f, 1.f, 0.f));
  //glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0.f, 1.f, 0.f));


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


  float increase_coeff = 10.f;
  if (near < 0.f)
    near *= increase_coeff;
  else
    near /= increase_coeff;
  if (far < 0.f)
    far /= increase_coeff;
  else
    far *= increase_coeff;


  /*
  float fbound = right - left;
  float worldUnitsPerTexel = fbound / (float)size_texture;

  left /= worldUnitsPerTexel;
  left = std::floor(left);
  left *= worldUnitsPerTexel;
  left = std::floor(left);

  right /= worldUnitsPerTexel;
  right = std::floor(right);
  right *= worldUnitsPerTexel;
  right = std::floor(right);

  fbound = top - bottom;
  worldUnitsPerTexel = fbound / (float)size_texture;

  bottom /= worldUnitsPerTexel;
  bottom = std::floor(bottom);
  bottom *= worldUnitsPerTexel;
  bottom = std::floor(bottom);

  top /= worldUnitsPerTexel;
  top = std::floor(top);
  top *= worldUnitsPerTexel;
  top = std::floor(top);

  near = std::floor(near);
  far = std::floor(far);

  if (i == 0)
    std::cout << left << ' ' << right << ' ' << bottom << ' ' << top << ' ' << near << ' ' << far << std::endl;
  */


  glm::mat4 lightProj = glm::ortho(left, right, bottom, top, near, far);

  return lightProj * lightView;
}