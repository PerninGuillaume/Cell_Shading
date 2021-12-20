#pragma once
#include "../program.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void set_zAtoon(program* program);
program *init_program(const std::string& vertex_shader_filename,
                      const std::string& fragment_shader_filename, const std::string& geometry_shader_filename = ""
                      , const std::map<std::string, std::string>& values_to_replace = {});
void renderQuad();
void renderQuad_corner();
unsigned int load_image(const std::string& file);
void display_glm_vec3(const glm::vec3& vec_3);
