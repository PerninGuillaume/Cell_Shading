#include <iostream>
#include "misc.h"
#include "stb_image.h"

void set_zAtoon(program* program) {
  float zAtoon_data[256] = {0};

  for (unsigned int i = 0; i < 256; ++i) {
    if (i <= 120)
      zAtoon_data[i] = 0.0f;
    else if (i <= 136)
      zAtoon_data[i] = ((i - 120) * 16) / 256.0f;
    else
      zAtoon_data[i] = 1.0f;
  }
/*
for (unsigned int i = 0; i < 256; ++i) {
  if (i <= 120)
    zAtoon_data[i] = 0.0f;
  else if (i <= 136)
    zAtoon_data[i] = ((i - 120) * 16) / 256.0f;
  else if (i <= 180)
    zAtoon_data[i] = ((i - 100) * 2) / 256.0f;
  else if (i <= 230)
    zAtoon_data[i] = ((i - 80) * 2) / 256.0f;
  else
    zAtoon_data[i] = 1.0f;
}
 */
  //Lookup table for the coeff variable in the fragment shader
  //It has the same name as the one used internally by zelda the wind waker
  program->set_uniform_vector_float("zAtoon", 256, zAtoon_data);

}

program *init_program(const std::string& vertex_shader_filename,
                      const std::string& fragment_shader_filename, const std::string& geometry_shader_filename
                      , const std::map<std::string, std::string>& values_to_replace) {
  program *program = program::make_program(vertex_shader_filename,
                                           fragment_shader_filename, geometry_shader_filename, values_to_replace);
  std::cout << program->get_log();
  if (!program->is_ready()) {
    throw "Program is not ready";
  }
  program->use();
  return program;
}

// Learnopengl function
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
  if (quadVAO == 0)
  {
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

unsigned int quadVAO_corner = 0;
unsigned int quadVBO_corner;
void renderQuad_corner()
{
  if (quadVAO_corner == 0)
  {
    float quadVertices[] = {
        // positions        // texture Coords
        -.2f,  1.0f, 0.0f, 0.0f, 1.0f,
        -.2f, 0.4f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.4f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO_corner);
    glGenBuffers(1, &quadVBO_corner);
    glBindVertexArray(quadVAO_corner);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO_corner);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  }
  glBindVertexArray(quadVAO_corner);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

unsigned int load_image(const std::string& file) {
  int width, height, nrChannels;
  unsigned char *data;

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else{
    std::cerr << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
  return texture;
}

void display_glm_vec3(const glm::vec3& vec_3) {
  std::cout << vec_3.x << ' ' << vec_3.y << ' ' << vec_3.z << std::endl;
}
