#include "Skybox.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iostream>
#include "../stb_image.h"
#include "program.h"
#include "misc.h"

unsigned int loadCubemap(const std::vector<std::string> &faces) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
      );
      stbi_image_free(data);
    } else {
      std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

unsigned int loadSkyBox(program *program) {

  // right left top bottom back front
  std::vector<std::string> faces = {
      "images/skybox/back.jpg", "images/skybox/back.jpg", "images/skybox/top.jpg", "images/skybox/back.jpg", "images/skybox/back.jpg", "images/skybox/back.jpg"
  };
  unsigned int cubemapTexture = loadCubemap(faces);
  program->set_uniform_int("skybox", 0);
  return cubemapTexture;
}

unsigned int skyBox_create_VAO() {
  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f
  };

  unsigned int skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
  return skyboxVAO;
}

std::vector<unsigned int> loadClouds()
{
  std::vector<unsigned int> clouds{};

  std::vector<std::string> files {"images/sprites/bigf_cloud.png",
      "images/sprites/bigf_cloud_mask.png",
      "images/sprites/longf_cloud.png",
      "images/sprites/longf_cloud_mask.png"};

  for (const auto &file : files) {
    clouds.emplace_back(load_image(file));
  }
  return clouds;
}


unsigned int clouds_create_VAO() {
  float r = -500;
  std::vector<float> angles = {};
  for (int i = 0; i < 4; i ++)
  {
    angles.push_back(i * 360 / 4);
  }

  float height = 5.0 * 10;

  std::vector<float> cloudsVerticesV = {};

  for (const auto & angle : angles)
  {

    float nb = rand() % 50;
    float nb2 = rand() % 50;
    float x_start = (r * cos((angle - nb) * PI / 180.0f));
    float x_end = (r * cos((angle + nb2) * PI / 180.0f));
    float y_start = (r * sin((angle - nb) * PI / 180.0f));
    float y_end = (r * sin((angle + nb2) * PI / 180.0f));

    std::vector<float> vertices = {
            // position                tex coords
            x_start,  -10.0f, y_end,            1.0f, 1.0f,
            x_end,  -10.0f, y_start,            0.0f, 1.0f,
            x_end,  height, y_start,            0.0f, 0.0f,
            x_end,  height, y_start,            0.0f, 0.0f,
            x_start,  height, y_end,            1.0f, 0.0f,
            x_start,  -10.0f, y_end,            1.0f, 1.0f
    };

    cloudsVerticesV.insert(cloudsVerticesV.end(), vertices.begin(), vertices.end());
  }

  unsigned int cloudsVAO, cloudsVBO;
  glGenVertexArrays(1, &cloudsVAO);
  glGenBuffers(1, &cloudsVBO);
  glBindVertexArray(cloudsVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cloudsVBO);
  glBufferData(GL_ARRAY_BUFFER, cloudsVerticesV.size() * sizeof(float), cloudsVerticesV.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  return cloudsVAO;
}

unsigned int loadWaves()
{
  std::string file = "images/sprites/wave1.png";

  return load_image(file);
}

unsigned int waves_create_VAO_vector(int nb_of_waves, std::vector<glm::vec3> &waves_center, const glm::vec3& center_of_waves) {
  std::vector<float> angles = {};
  for (int i = 0; i < nb_of_waves; i ++)
  {
    angles.push_back(i * 360 / nb_of_waves);
  }

  float height = -1.0;
  float base_height = -2.0f;
  float middle_height = -10.0f;

  for (const auto & angle : angles) {
    float nb = rand() % 50;
    float nb2 = rand() % 50;
    int radius_max = 500;
    int radius_min = 75;
    float r = -1.0f * (rand() % (radius_max - radius_min + 1) + radius_min);

    float x_start = (r * cos((angle - nb) * PI / 180.0f));
    float x_end = (r * cos((angle + nb2) * PI / 180.0f));
    float y_start = (r * sin((angle - nb) * PI / 180.0f));
    float y_end = (r * sin((angle + nb2) * PI / 180.0f));
    glm::vec3 center_wave = glm::vec3((x_end + x_start) / 2.0f, middle_height, (y_end + y_start) / 2.0f);
    waves_center.push_back(center_wave + center_of_waves);

  }

  std::vector<float> vertices = {
      // position                tex coords
      -0.5f, base_height, 0.5f, 1.0f, 1.0f, 0.5f, base_height, -0.5f, 0.0f, 1.0f, 0.5f, height, -0.5f, 0.0f, 0.0f, 0.5f, height, -0.5f, 0.0f, 0.0f, -0.5f, height, 0.5f, 1.0f, 0.0f, -0.5f, base_height, 0.5f, 1.0f, 1.0f
  };
  unsigned int waveVAO, waveVBO;
  glGenVertexArrays(1, &waveVAO);
  glGenBuffers(1, &waveVBO);
  glBindVertexArray(waveVAO);
  glBindBuffer(GL_ARRAY_BUFFER, waveVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  return waveVAO;
}