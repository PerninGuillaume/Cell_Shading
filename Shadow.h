#pragma once
#include "../program.h"

class Shadow {
 public:
  Shadow(unsigned int shadow_width = 2048, unsigned int shadow_height = 2048);


  void generate_depth_map_texture();
  void generate_depth_map_frame_buffer();

  program* shadow_shader_depth;
  unsigned int depthMapTexture;
  unsigned int depthMapFBO;
  unsigned int shadow_width;
  unsigned int shadow_height;
};
