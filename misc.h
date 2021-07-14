#include "../program.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void set_zAtoon(program* program);
program *init_program(const std::string& vertex_shader_filename,
                      const std::string& fragment_shader_filename, const std::string& geometry_shader_filename = "");
void renderQuad();
unsigned int load_image(const std::string& file);
