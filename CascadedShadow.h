#pragma once
#include <memory>
#include "../program.h"
#include "../examples/windfall.h"
#include "Model.h"
#include "Camera.h"

class CascadedShadow {
 public:
  explicit CascadedShadow(unsigned int nb_division, unsigned int shadow_width, unsigned int shadow_height);

  void generate_depth_map_texture();
  void generate_depth_map_frame_buffer();
  std::vector<glm::mat4> computeShadowCascaded(const windfall::Param& params, std::shared_ptr<Camera> camera
      , Model& windfall_lowres, int SRC_WIDTH, int SRC_HEIGHT
      , const glm::mat4& view, const glm::vec3& lightDir, const glm::mat4& model_mat_windfall, GLuint waterVAO);

  program* shadow_shader_depth;
  unsigned int depthMapFBO{};
  std::vector<unsigned int> depthMapTextures;
  unsigned int nb_division;
  unsigned int shadow_width;
  unsigned int shadow_height;
  std::vector<glm::mat4> lightSpaceMatrices;
  std::vector<float> cascades_delimitations = {0.1f, 20.0f, 100.0f, 250.0f};

};

std::vector<glm::vec4> get_frustrum_world_space_coordinates(const glm::mat4& view, const glm::mat4& projection);
glm::mat4 computeLightViewProjMatrix(const glm::vec3& lightDir, const glm::mat4& view
    , const glm::mat4& projection, float increase_coeff_z, float increase_coeff_xy);
