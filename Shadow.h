#pragma once
#include "../program.h"

class Shadow {
 public:
  Shadow(unsigned int shadow_width = 2048, unsigned int shadow_height = 2048);


  unsigned int generate_depth_map_texture();
  unsigned int generate_depth_map_frame_buffer();
  unsigned int depthMapTexture;
  unsigned int depthMapFBO;
  unsigned int shadow_width;
  unsigned int shadow_height;
};