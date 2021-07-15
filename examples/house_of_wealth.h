#pragma once

#include "../program.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace house_of_wealth {
void display(GLFWwindow* window, bool use_hd_texture = true);
}