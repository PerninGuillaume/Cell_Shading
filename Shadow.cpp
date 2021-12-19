#include <iomanip>
#include <iostream>

#include "Shadow.h"
#include "misc.h"

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
