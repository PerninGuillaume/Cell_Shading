#pragma once

#define PI 3.14159265

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iostream>
#include "../stb_image.h"
#include "program.h"

unsigned int loadCubemap(const std::vector<std::string> &faces);

unsigned int loadSkyBox(program *program);

unsigned int skyBox_create_VAO();

std::vector<unsigned int> loadClouds();

unsigned int clouds_create_VAO();

std::pair<unsigned int, std::vector<unsigned int>> waves_create_VAO_vector(int nb_of_waves, std::vector<glm::vec3> &waves_center, const glm::vec3& center_of_waves);

unsigned int loadWaves();
