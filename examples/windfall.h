#pragma once

#include "../program.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace windfall {
void display(GLFWwindow* window, bool load_hd_texture = false);
}