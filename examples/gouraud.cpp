#include <iostream>
#include "gouraud.h"
namespace gouraud {
void processInput_gouraud(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

unsigned int init_gouraud(program *program) {

  //position and colors and normals
  float vertices[] = {
      -0.5f, -0.5f, 0.0f, 1.f, 0.f, 0.f, -1.0f, -1.0f, 1.0f, // left
      0.5f, -0.5f, 0.0f, 0.f, 1.f, 0.f, 1.0f, -1.0f, 1.0f, // right
      0.0f, 0.5f, 0.0f, 0.f, 0.f, 1.f, 0.0f, 1.0f, 1.0f  // top
  };

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *) 0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *) (6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  program->set_uniform_vec3("color_uniform", 0.0f, 0.5f, 0.0f);
  program->set_uniform_vec3("light_position", 0.0f, 0.0f, 1.0f);
  program->set_uniform_float("light_intensity", 10.0f);

  return VAO;
}

unsigned int gouraud() {
  program *program = program::make_program("shaders/vertex_gouraud.glsl",
                                           "shaders/fragment_gouraud.glsl");
  std::cout << program->get_log();
  if (!program->is_ready()) {
    throw "Program is not ready";
  }
  return init_gouraud(program);
}

void display(GLFWwindow *window) {
  unsigned int VAO = gouraud();

  while (!glfwWindowShouldClose(window)) {

    processInput_gouraud(window);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glfwTerminate();

}
}