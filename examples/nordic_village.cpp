#include "nordic_village.h"

#include "../Camera.h"
#include <iostream>

#include "../stb_image.h"
#include "../Model.h"

namespace nordic_village {

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

  program *program = init_program(window, "shaders/vertex_link.glsl",
                                      "shaders/fragment_link.glsl");

  stbi_set_flip_vertically_on_load(true);
  Model village("models/nordic-village/source/final_poblado.obj");
  //Model village("models/cube.fbx");

  glEnable(GL_DEPTH_TEST);
  //Capture the mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  //DirLight
  program->set_uniform_vec3("dirLight.direction", -0.3f, -0.7f, -0.3f);
  program->set_uniform_vec3("dirLight.ambient",  2.0f);
  program->set_uniform_vec3("dirLight.specular", 0.2f);
  auto diffuseColor = glm::vec3(1.0f);
  program->set_uniform_vec3("dirLight.diffuse", diffuseColor); // darken diffuse light a bit
  float zAtoon_data[256] = {0};

  for (unsigned int i = 0; i < 256; ++i) {
    if (i <= 120)
      zAtoon_data[i] = 0.0f;
    else if (i <= 136)
      zAtoon_data[i] = ((i - 120) * 16) / 256.0f;
    else if (i <= 180)
      zAtoon_data[i] = ((i - 100) * 2) / 256.0f;
    else if (i <= 230)
      zAtoon_data[i] = ((i - 80) * 2) / 256.0f;
    else
      zAtoon_data[i] = 1.0f;
  }

  program->set_uniform_vector_float("zAtoon", 256, zAtoon_data);

  while (!glfwWindowShouldClose(window)) {
    std::cout << displacement << std::endl;
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    deltaTime_since_last_press = currentFrame - time_of_last_press;
    lastFrame = currentFrame;

    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera.fov_camera), 800.0f / 600.0f,
                                            0.1f, 100.0f);
    glm::mat4 view = camera.view_matrix();

    program->set_uniform_mat4("view", view);
    program->set_uniform_mat4("projection", projection);

    //program->set_uniform_vec3("viewPosition", camera.position);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -10.0f, -3.0f));
//    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    program->set_uniform_mat4("model", model);

    std::vector<glm::vec3> offset_vecs{camera.right, -camera.right, camera.up, -camera.up};

//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_FRONT);

//    if (wireframe)
//      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//
//
//    if (wireframe)
//      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    glCullFace(GL_BACK);
    village.draw(program);

    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glfwTerminate();

}
}
