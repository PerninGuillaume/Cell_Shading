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

unsigned int waves_create_VAO(glm::vec3 position_camera);
std::vector<unsigned int> waves_create_VAO_vector(glm::vec3 position_camera, std::vector<glm::vec3> &waves_center, glm::vec3 center_of_waves);

std::vector<unsigned int> loadWaves();
