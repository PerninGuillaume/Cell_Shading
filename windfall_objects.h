#pragma once
#include "program.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

unsigned int wind_create_VAO();
unsigned int water_create_VAO(float height_sea);
std::vector<unsigned int> loadSunTextures();
unsigned int sun_create_VAO(float angle);
float get_eye_cancer(float alignment, float alignment_limit, float lowest_eye_cancer);
std::vector<unsigned int> loadShore();
unsigned int shore_create_VAO(int &nb_of_points, float height_sea);
