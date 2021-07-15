#pragma once
#include "program.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

unsigned int water_create_VAO();
std::vector<unsigned int> loadSunTextures();
unsigned int sun_create_VAO();
std::vector<unsigned int> loadShore();
unsigned int shore_create_VAO(int &nb_of_points);
