#pragma once
#include "../program.h"

class Shadow {
 public:
  Shadow();


  unsigned int generate_depth_map_texture();
  unsigned int generate_depth_map_frame_buffer();
  unsigned int depthMapTexture;
  unsigned int depthMapFBO;
  unsigned int SHADOW_WIDTH = 1024;
  unsigned int SHADOW_HEIGHT = 1024;
};