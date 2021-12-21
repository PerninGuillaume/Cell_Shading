#pragma once
#include "../program.h"
#include "Model.h"
#include "../examples/windfall.h"

class Shadow {
 public:
  Shadow(unsigned int shadow_width = 2048, unsigned int shadow_height = 2048);


  void generate_depth_map_texture();
  void generate_depth_map_frame_buffer();
  std::vector<glm::mat4> computeShadow(const windfall::Param& params, Model& windfall_lowres, int SRC_WIDTH, int SRC_HEIGHT
      , const glm::mat4& view, const glm::mat4& projection, const glm::vec3& lightDir, const glm::mat4& model_mat_windfall);


    program* shadow_shader_depth;
  unsigned int depthMapTexture;
  unsigned int depthMapFBO;
  unsigned int shadow_width;
  unsigned int shadow_height;
};
