#include "bomb_shop.h"

#include "../Camera.h"
#include <iostream>

#include "../stb_image.h"
#include "../Model.h"

namespace bomb_shop {

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastXMouse = 800 / 2;
float lastYMouse = 600 / 2;
float rotation = 0;
bool wireframe = false;
bool vertex_shifted_along_normal = false;
float deltaTime_since_last_press = 0.0f;
float time_of_last_press = 0.0f;
float displacement = 0.006f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

void processInput(GLFWwindow *window) {

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    camera.shift_pressed(true);
  else
    camera.shift_pressed(false);

  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    camera.ctrl_pressed(true);
  else
    camera.ctrl_pressed(false);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    rotation += 2.0f;
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    rotation -= 2.0f;

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    displacement += 0.00001f;
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    displacement -= 0.00001f;

  if (glfwGetKey(window, GLFW_KEY_P) && deltaTime_since_last_press > 0.2f) {
    time_of_last_press = glfwGetTime();
    wireframe = !wireframe;
  }
  if (glfwGetKey(window, GLFW_KEY_N) && deltaTime_since_last_press > 0.2f) {
    time_of_last_press = glfwGetTime();
    vertex_shifted_along_normal = !vertex_shifted_along_normal;
  }
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

  program *program = init_program(window, "shaders/vertex_model.glsl",
                                  "shaders/fragment_model.glsl");

  Model village("models/Bomb_Shop/Bomb shop.obj");

  glEnable(GL_DEPTH_TEST);
  //Capture the mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  while (!glfwWindowShouldClose(window)) {

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

   processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera.fov_camera), 800.0f / 600.0f,
                                            0.01f, 1000.0f);
    glm::mat4 view = camera.view_matrix();

    program->set_uniform_mat4("view", view);
    program->set_uniform_mat4("projection", projection);

    //program->set_uniform_vec3("viewPosition", camera.position);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    program->set_uniform_mat4("model", model);

    village.draw(program);

    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glfwTerminate();

}
}

