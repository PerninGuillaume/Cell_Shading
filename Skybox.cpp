#include "Skybox.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <iostream>
#include "../stb_image.h"
#include "program.h"

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


  int width, height, nrChannels;
  unsigned char *data;

  for (const auto &file : files)
  {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    clouds.emplace_back(texture);
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

  float width = 10.0 * 50;
  float height = 5.0 * 10;

  std::vector<float> cloudsVerticesV = {};

  for (const auto & angle : angles)
  {

    float nb = rand() % 50;
    float nb2 = rand() % 50;
//    std::cout << angle << std::endl;
    float x_start = (r * cos((angle - nb) * PI / 180.0f));
    float x_end = (r * cos((angle + nb2) * PI / 180.0f));
    float y_start = (r * sin((angle - nb) * PI / 180.0f));
    float y_end = (r * sin((angle + nb2) * PI / 180.0f));

//    std::cout << x_start << " / " << y_start << "  |  " << x_end << " / " << y_end << std::endl;

//    float offset = width / 2;
//



//    float x_end = x + offset / 2;
//    float x_start = x - width / 2;

//    float norm = sqrt(x*x + y*y);
//    float x_start = x - offset * -y / norm;
//    float y_start = y - offset * x / norm;
//    float x_end = x + offset * -y / norm;
//    float y_end = y + offset * x / norm;

//    float y_end = y + width / 2 * cos(180 - angle_deg);
//    float y_start = y + width / 2 * sin(180 - angle_deg);
//    float norm = sqrt(x*x + y*y);

//    x_start = 5;
//    y_start = 5;
//    x_end = 10;
//    y_end = 10;
//    height = 0;

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


//  float cloudsVertices[] = {
//      // position                tex coords
//      0.0f,  -10.0f, r,    1.0f, 1.0f,
//      width,  -10.0f, r,    0.0f, 1.0f,
//      width,  height, r,    0.0f, 0.0f,
//      width,  height, r,    0.0f, 0.0f,
//      0.0f,  height,r,    1.0f, 0.0f,
//      0.0f,  -10.0f, r,   1.0f, 1.0f,
//
//      0.0f,  -10.0f, -r,    1.0f, 1.0f,
//      width,  -10.0f, -r,    0.0f, 1.0f,
//      width,  height, -r,    0.0f, 0.0f,
//      width,  height, -r,    0.0f, 0.0f,
//      0.0f,  height,-r,    1.0f, 0.0f,
//      0.0f,  -10.0f, -r,   1.0f, 1.0f,
//  };

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

std::vector<unsigned int> loadWaves()
{
  std::vector<unsigned int> waves{};

  std::vector<std::string> files {
    "images/sprites/wave1.png",
      };


  int width, height, nrChannels;
  unsigned char *data;

  for (const auto &file : files)
  {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    waves.emplace_back(texture);
  }
  return waves;
}

std::vector<unsigned int> waves_create_VAO_vector(glm::vec3 position_camera, std::vector<glm::vec3> &waves_center, glm::vec3 center_of_waves) {
  std::vector<unsigned int> waves_VAO;
  float r = -50;
  std::vector<float> angles = {};
  for (int i = 0; i < 4; i ++)
  {
    angles.push_back(i * 360 / 4);
  }

  float height = -1.0;
  float base_height = -2.0f;
  float middle_height = -10.0f;

  for (const auto & angle : angles)
  {

    //float nb = rand() % 50;
    //float nb2 = rand() % 50;
    float nb = 4;
    float nb2 = 8;
//    std::cout << angle << std::endl;
    float x_start = (r * cos((angle - nb) * PI / 180.0f));
    float x_end = (r * cos((angle + nb2) * PI / 180.0f));
    float y_start = (r * sin((angle - nb) * PI / 180.0f));
    float y_end = (r * sin((angle + nb2) * PI / 180.0f));
    glm::vec3 center_wave = glm::vec3((x_end + x_start) / 2.0f, middle_height, (y_end + y_start) / 2.0f);
    //glm::vec3 center_wave = glm::vec3((x_end + x_start) / 2.0f, (height + base_height) / 2.0f, (y_end + y_start) / 2.0f);
    waves_center.push_back(center_wave + center_of_waves);

    /*
    std::vector<float> vertices = {
        // position                tex coords
        x_start,  base_height, y_end,            1.0f, 1.0f,
        x_end,  base_height, y_start,            0.0f, 1.0f,
        x_end,  height, y_start,            0.0f, 0.0f,
        x_end,  height, y_start,            0.0f, 0.0f,
        x_start,  height, y_end,            1.0f, 0.0f,
        x_start,  base_height, y_end,            1.0f, 1.0f
    };*/

    std::vector<float> vertices = {
        // position                tex coords
        -0.5f,  base_height, 0.5f,            1.0f, 1.0f,
        0.5f,  base_height, -0.5f,            0.0f, 1.0f,
        0.5f,  height, -0.5f,            0.0f, 0.0f,
        0.5f,  height, -0.5f,            0.0f, 0.0f,
        -0.5f,  height, 0.5f,            1.0f, 0.0f,
        -0.5f,  base_height, 0.5f,            1.0f, 1.0f
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

    waves_VAO.push_back(waveVAO);

  }

  return waves_VAO;
}
/*
void waves_instance() {
  unsigned int instanceVBO
}*/