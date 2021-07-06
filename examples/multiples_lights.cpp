#include "multiples_lights.h"
#include "../Camera.h"
#include <iostream>

#include "../stb_image.h"
namespace multiples_lights {

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastXMouse = 800 / 2;
float lastYMouse = 600 / 2;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

void processInput(GLFWwindow *window) {

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastXMouse = xpos;
    lastYMouse = ypos;
    firstMouse = false;
  }

  double xoffset = xpos - lastXMouse;
  double yoffset = lastYMouse - ypos;
  lastXMouse = xpos;
  lastYMouse = ypos;

  camera.processMouse(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.processScroll(yoffset);
}

unsigned int loadTexture(char const * path)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

std::vector<unsigned int> init_VAOs() {
  float vertices_3D[] = {
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
      0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
      0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
      0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
      0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
      -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
      0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
      0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
      0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
  };

  unsigned int cubeVAO, VBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(cubeVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_3D), vertices_3D, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  unsigned int lightVAO;
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  return std::vector<unsigned int>{cubeVAO, lightVAO};
}

program *init_program(GLFWwindow *window, const std::string& vertex_shader_filename,
                      const std::string& fragment_shader_filename) {
  program *program = program::make_program(vertex_shader_filename,
                                           fragment_shader_filename);
  std::cout << program->get_log();
  if (!program->is_ready()) {
    throw "Program is not ready";
  }
  program->use();
  return program;
}

void display_multiples_lights(program *programCube, program *programLight, glm::vec3 pointLightPositions[2]) {

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //DirLight
  programCube->set_uniform_vec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
  programCube->set_uniform_vec3("dirLight.ambient",  0.2f);
  programCube->set_uniform_vec3("dirLight.specular", 1.0f);
  auto diffuseColor = glm::vec3(0.5f);
  programCube->set_uniform_vec3("dirLight.diffuse", diffuseColor); // darken diffuse light a bit
  programLight->set_uniform_vec3("light", diffuseColor);

  // point light 1
  programCube->set_uniform_vec3("pointLights[0].position", pointLightPositions[0]);
  programCube->set_uniform_vec3("pointLights[0].ambient", 0.05f);
  programCube->set_uniform_vec3("pointLights[0].diffuse", 0.8f);
  programCube->set_uniform_vec3("pointLights[0].specular", 1.0f);
  programCube->set_uniform_float("pointLights[0].constant", 1.0f);
  programCube->set_uniform_float("pointLights[0].linear", 0.09);
  programCube->set_uniform_float("pointLights[0].quadratic", 0.032);

  // point light 2

  programCube->set_uniform_vec3("pointLights[1].position", pointLightPositions[1]);
  programCube->set_uniform_vec3("pointLights[1].ambient", 0.05f);
  programCube->set_uniform_vec3("pointLights[1].diffuse", 0.8f);
  programCube->set_uniform_vec3("pointLights[1].specular", 1.0f);
  programCube->set_uniform_float("pointLights[1].constant", 1.0f);
  programCube->set_uniform_float("pointLights[1].linear", 0.09);
  programCube->set_uniform_float("pointLights[1].quadratic", 0.032);


  // flash light
  programCube->set_uniform_vec3("flashLight.position", camera.position);
  programCube->set_uniform_vec3("flashLight.direction", camera.front);
  programCube->set_uniform_float("flashLight.cutOff", glm::cos(glm::radians(12.5f)));
  programCube->set_uniform_float("flashLight.outerCutOff", glm::cos(glm::radians(15.0f)));
  programCube->set_uniform_vec3("flashLight.ambient", 0.0f);
  programCube->set_uniform_vec3("flashLight.diffuse", 1.0f);
  programCube->set_uniform_vec3("flashLight.specular", 1.0f);
  programCube->set_uniform_float("flashLight.constant", 1.0f);
  programCube->set_uniform_float("flashLight.linear", 0.09f);
  programCube->set_uniform_float("flashLight.quadratic", 0.032f);

}

void display_horror_settings(program *programCube, program *programLight, glm::vec3 pointLightPositions[2]) {

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //DirLight
  programCube->set_uniform_vec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
  programCube->set_uniform_vec3("dirLight.ambient",  0.0f);
  programCube->set_uniform_vec3("dirLight.specular", 0.2f);
  auto diffuseColor = glm::vec3(0.05f);
  programCube->set_uniform_vec3("dirLight.diffuse", diffuseColor); // darken diffuse light a bit

  // point light 1

  programCube->set_uniform_vec3("pointLights[0].position", pointLightPositions[0]);
  programCube->set_uniform_vec3("pointLights[0].ambient", 0.1f);
  programCube->set_uniform_vec3("pointLights[0].diffuse", 0.2f, 0.0f, 0.0f);
  programCube->set_uniform_vec3("pointLights[0].specular", 0.1f);
  programCube->set_uniform_float("pointLights[0].constant", 1.0f);
  programCube->set_uniform_float("pointLights[0].linear", 0.22);
  programCube->set_uniform_float("pointLights[0].quadratic", 0.20);

  programLight->set_uniform_vec3("light", 0.5f, 0.0f, 0.0f);

  // point light 2

  programCube->set_uniform_vec3("pointLights[1].position", pointLightPositions[1]);
  programCube->set_uniform_vec3("pointLights[1].ambient", 0.1f);
  programCube->set_uniform_vec3("pointLights[1].diffuse", 0.2f, 0.0f, 0.0f);
  programCube->set_uniform_vec3("pointLights[1].specular", 0.1f);
  programCube->set_uniform_float("pointLights[1].constant", 1.0f);
  programCube->set_uniform_float("pointLights[1].linear", 0.22);
  programCube->set_uniform_float("pointLights[1].quadratic", 0.20);


  // flash light
  programCube->set_uniform_vec3("flashLight.position", camera.position);
  programCube->set_uniform_vec3("flashLight.direction", camera.front);
  programCube->set_uniform_float("flashLight.cutOff", glm::cos(glm::radians(10.0f)));
  programCube->set_uniform_float("flashLight.outerCutOff", glm::cos(glm::radians(15.0f)));
  programCube->set_uniform_vec3("flashLight.ambient", 0.0f);
  programCube->set_uniform_vec3("flashLight.diffuse", 1.0f);
  programCube->set_uniform_vec3("flashLight.specular", 1.0f);
  programCube->set_uniform_float("flashLight.constant", 1.0f);
  programCube->set_uniform_float("flashLight.linear", 0.22f);
  programCube->set_uniform_float("flashLight.quadratic", 0.20f);

}

void display(GLFWwindow *window) {

  program *programCube = init_program(window, "shaders/vertex_multiples_lights.glsl",
                                      "shaders/fragment_multiples_lights.glsl");
  program *programLight = init_program(window, "shaders/vertex_colors.glsl",
                                       "shaders/fragment_light.glsl");
  unsigned int diffuseMap = loadTexture("images/container2.png");
  unsigned int specularMap = loadTexture("images/container2_specular.png");
  std::vector<unsigned int> VAOs = init_VAOs();
  unsigned int cubeVAO = VAOs[0];
  unsigned int lightVAO = VAOs[1];

  glEnable(GL_DEPTH_TEST);
  //Capture the mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  programCube->set_uniform_int("material.diffuse", 0);
  programCube->set_uniform_int("material.specular", 1);

  glm::vec3 cubePositions[] = {
      glm::vec3( 0.0f,  0.0f,  0.0f),
      glm::vec3( 2.0f,  5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f),
      glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3( 2.4f, -0.4f, -3.5f),
      glm::vec3(-1.7f,  3.0f, -7.5f),
      glm::vec3( 1.3f, -2.0f, -2.5f),
      glm::vec3( 1.5f,  2.0f, -2.5f),
      glm::vec3( 1.5f,  0.2f, -1.5f),
      glm::vec3(-1.3f,  1.0f, -1.5f)
  };

  glm::vec3 pointLightPositions[] = {
      glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f)
  };

  while (!glfwWindowShouldClose(window)) {

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window); //input

    //display_multiples_lights(programCube, programLight, pointLightPositions);
    display_horror_settings(programCube, programLight, pointLightPositions);

    glm::mat4 projection = glm::perspective(glm::radians(camera.fov_camera), 800.0f / 600.0f,
                                            0.1f, 100.0f);
    glm::mat4 view = camera.view_matrix();
    programCube->set_uniform_mat4("view", view);
    programCube->set_uniform_mat4("projection", projection);

    programCube->set_uniform_vec3("viewPosition", camera.position);

    programCube->set_uniform_float("material.shininess", 64.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);

    glBindVertexArray(cubeVAO);
    for (unsigned int i = 0; i < 10; ++i) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, cubePositions[i]);

      float angle = 20.0f * i;
      model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      programCube->set_uniform_mat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    programLight->set_uniform_mat4("view", view);
    programLight->set_uniform_mat4("projection", projection);

    glBindVertexArray(lightVAO);
    for (unsigned int i = 0; i < 2; ++i) {
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, pointLightPositions[i]);
      model = glm::scale(model, glm::vec3(0.2f));
      programLight->set_uniform_mat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);

    }



    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glfwTerminate();

}
}