#include "colors.h"
#include "../Camera.h"
#include <iostream>

#include "../stb_image.h"
namespace colors {

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

std::vector<unsigned int> init_VAOs() {
  float vertices_3D[] = {
          -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
          -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

          -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
          -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
          -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

          -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
          -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

          0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
          0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
          0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
          0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
          0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
          -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

          -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
          -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
          -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
  };

  unsigned int cubeVAO, VBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(cubeVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_3D), vertices_3D, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  unsigned int lightVAO;
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
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

void display(GLFWwindow *window) {

  program *programCube = init_program(window, "shaders/vertex_colors.glsl",
      "shaders/fragment_colors.glsl");
  program *programLight = init_program(window, "shaders/vertex_colors.glsl",
      "shaders/fragment_light.glsl");
  std::vector<unsigned int> VAOs = init_VAOs();
  unsigned int cubeVAO = VAOs[0];
  unsigned int lightVAO = VAOs[1];

  glEnable(GL_DEPTH_TEST);
  //Capture the mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glm::vec3 lightPos(1.2f, 1.0f, -2.0f);
  while (!glfwWindowShouldClose(window)) {

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 rotation = glm::mat4(1.0f);
    //float rotation_angle = fmod(glm::radians(glfwGetTime()), 6.28f);
    //float rotation_angle = 0.01f;
    //lightPos = glm::rotate(rotation, rotation_angle, glm::vec3(0.0f, 1.0f, 0.0f))
    //        * glm::vec4(lightPos, 1.0f);

    glm::mat4 projection = glm::perspective(glm::radians(camera.fov_camera), 800.0f / 600.0f,
                                            0.1f, 100.0f);
    glm::mat4 view = camera.view_matrix();

    programCube->use();
    programCube->set_uniform_vec3("lightPosition", lightPos);
    programCube->set_uniform_vec3("viewPosition", camera.position);

    programCube->set_uniform_mat4("view", view);
    programCube->set_uniform_mat4("projection", projection);
    programCube->set_uniform_mat4("model", glm::mat4(1.0f));

    bool discotheque = false;
    bool cyan_plastic = false;
    if (cyan_plastic) {
      programCube->set_uniform_vec3("material.ambient", 0.0f, 0.1f, 0.06f);
      programCube->set_uniform_vec3("material.diffuse", 0.0f, 0.50980392f, 0.50980392);
      programCube->set_uniform_vec3("material.specular", 0.50196078f,	0.50196078f, 0.50196078f);
      programCube->set_uniform_float("material.shininess", 32.0f);
      //programCube->set_uniform_float("material.shininess", 25.0f);
    }
    else {
      programCube->set_uniform_vec3("material.ambient", 1.0f, 0.5f, 0.31f);
      programCube->set_uniform_vec3("material.diffuse", 1.0f, 0.5f, 0.31f);
      programCube->set_uniform_vec3("material.specular", 0.5f, 0.5f, 0.5f);
      programCube->set_uniform_float("material.shininess", 32.0f);
    }
    if (discotheque) {
      glm::vec3 lightColor;
      lightColor.x = sin(glfwGetTime() * 2.0f);
      lightColor.y = sin(glfwGetTime() * 0.7f);
      lightColor.z = sin(glfwGetTime() * 1.3f);
      glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
      glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
      programCube->set_uniform_vec3("light.ambient", ambientColor);
      programCube->set_uniform_vec3("light.diffuse", diffuseColor);

      programLight->set_uniform_vec3("light", diffuseColor);
    } else {
      glm::vec3 ambientColor;
      glm::vec3 diffuseColor;
      if (cyan_plastic) {
        ambientColor = glm::vec3(1.0f);
        diffuseColor = glm::vec3(1.0f);
      }
      else {
        ambientColor = glm::vec3(0.2f);
        diffuseColor = glm::vec3(0.5f);
      }
      programCube->set_uniform_vec3("light.ambient",  ambientColor);

      programCube->set_uniform_vec3("light.diffuse", diffuseColor); // darken diffuse light a bit
      programLight->set_uniform_vec3("light", diffuseColor);
    }


    programCube->set_uniform_vec3("light.specular", 1.0f, 1.0f, 1.0f);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    programLight->use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    programLight->set_uniform_mat4("model", model);
    programLight->set_uniform_mat4("view", view);
    programLight->set_uniform_mat4("projection", projection);

    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);



    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glfwTerminate();

}

}