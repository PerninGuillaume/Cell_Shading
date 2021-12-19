#pragma once
#include "../program.h"

class CascadedShadow {
 public:
  CascadedShadow(unsigned int nb_division, unsigned int shadow_width = 1024, unsigned int shadow_height = 1024);

  void generate_depth_map_texture();
  void generate_depth_map_frame_buffer();

  program* shadow_shader_depth;
  unsigned int depthMapFBO;
  std::vector<unsigned int> depthMapTextures;
  unsigned int nb_division;
  unsigned int shadow_width;
  unsigned int shadow_height;
  std::vector<glm::mat4> lightSpaceMatrix;

};

std::vector<glm::vec4> get_frustrum_world_space_coordinates(const glm::mat4& view, const glm::mat4& projection);
glm::mat4 computeLightViewProjMatrix(const glm::vec3& lightPos, const glm::vec3& lightDir, const glm::mat4& view, const glm::mat4& projection);
