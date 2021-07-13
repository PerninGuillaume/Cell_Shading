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
