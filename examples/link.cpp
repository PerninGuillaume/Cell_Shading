#include <iostream>

#include "link.h"
#include "../stb_image.h"
#include "../Model.h"
#include "../Camera.h"
#include "../Shadow.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace link {

unsigned int SRC_WIDTH = 800.0f;
unsigned int SRC_HEIGHT = 600.0f;
bool use_im_gui = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastXMouse = (float)SRC_WIDTH / 2;
float lastYMouse = (float)SRC_HEIGHT / 2;
bool right_button_mouse_clicked = false;

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

  if (!right_button_mouse_clicked && use_im_gui)
    return;
  camera.processMouse(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.processScroll(yoffset);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    right_button_mouse_clicked = true;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    right_button_mouse_clicked = false;
  }
}

void set_zAtoon(program* program) {
  float zAtoon_data[256] = {0};

  for (unsigned int i = 0; i < 256; ++i) {
    if (i <= 120)
      zAtoon_data[i] = 0.0f;
    else if (i <= 136)
      zAtoon_data[i] = ((i - 120) * 16) / 256.0f;
    else
      zAtoon_data[i] = 1.0f;
  }

  //Lookup table for the coeff variable in the fragment shader
  //It has the same name as the one used internally by zelda the wind waker
  program->set_uniform_vector_float("zAtoon", 256, zAtoon_data);

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
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
  if (quadVAO == 0)
  {
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}
void display(GLFWwindow *window) {

  int width, height;
  glfwGetWindowSize(window, &width, &height);
  SRC_WIDTH = width;
  SRC_HEIGHT = height;

  if (use_im_gui) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
  }
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

// Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  // Setup Platfrom/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 450");
  ImGui::StyleColorsDark();

  program *shader = init_program(window, "shaders/vertex_link.glsl",
                                      "shaders/fragment_link.glsl");
  program *programOutline = init_program(window, "shaders/vertex_scale_up.glsl",
                                         "shaders/fragment_black.glsl");
  program *shadow_shader_depth = init_program(window, "shaders/vertex_shadow_depth.glsl",
                                              "shaders/fragment_shadow_depth.glsl");

  program *quad_depth_shader = init_program(window, "shaders/vertex_normalized_coord.glsl", "shaders/fragment_quad_depth.glsl");
  std::vector<program*> programs = {shader, programOutline};

  Model link("models/link-cartoon/source/LinkCartoon.fbx");
  Shadow shadow = Shadow();

  glEnable(GL_DEPTH_TEST);

  //DirLight
  shader->set_uniform_vec3("dirLight.direction", -0.3f, -1.0f, -0.3f);

  glm::vec3 lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);//Need a position for shadow

  set_zAtoon(shader);

  quad_depth_shader->set_uniform_int("depthMap", 0);
  shader->set_uniform_int("shadowMap", 0);
  bool use_zAtoon = true;
  bool use_shadow = true;
  bool contour_wireframe= false;
  bool vertex_shifted_along_normal = false;
  float rotation = 0;
  float displacement = 0.006f;
  float light_diffuse = 1.0f;
  float light_ambient = 0.5f;

  while (!glfwWindowShouldClose(window)) {
    if (use_im_gui) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
    }

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -2.0f, -3.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    // 1. Render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 7.5f;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;



    glViewport(0, 0, shadow.SHADOW_WIDTH, shadow.SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    //Render link
    shadow_shader_depth->set_uniform_mat4("model", model);
    shadow_shader_depth->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);

    link.draw(shadow_shader_depth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // reset viewport

    glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // 2. render scene as normal using the generated depth/shadow map
    // --------------------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(camera.fov_camera), (float)SRC_WIDTH / (float)SRC_HEIGHT,
                                            0.1f, 100.0f);
    glm::mat4 view = camera.view_matrix();

    for (auto prg : programs)
    {
        prg->set_uniform_mat4("view", view);
        prg->set_uniform_mat4("projection", projection);
    }


    shader->set_uniform_mat4("model", model);
    shader->set_uniform_bool("use_zAtoon", use_zAtoon);
    shader->set_uniform_bool("use_shadow", use_shadow);
    shader->set_uniform_vec3("dirLight.ambient",  light_ambient);
    shader->set_uniform_vec3("dirLight.diffuse", light_diffuse); // darken diffuse light a bit
    //shader->set_uniform_vec3("viewPos", camera.position);
    shader->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);



    std::vector<glm::vec3> offset_vecs{camera.right, -camera.right, camera.up, -camera.up};

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    if (contour_wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (vertex_shifted_along_normal) {
      programOutline->set_uniform_mat4("model", model);
      programOutline->set_uniform_float("displacement", displacement);
      link.draw(programOutline);
    }
    else {
      programOutline->set_uniform_float("displacement", 0.0f);
      for (const auto &dir : offset_vecs) {
        auto tmp_model = glm::translate(model, dir * 0.008f);
        programOutline->set_uniform_mat4("model", tmp_model);
        link.draw(programOutline);
      }
    }

    if (contour_wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glCullFace(GL_BACK);

    link.draw(shader);

// render Depth map to quad for visual debugging
    // ---------------------------------------------
    quad_depth_shader->use();
    quad_depth_shader->set_uniform_float("near_plane", near_plane);
    quad_depth_shader->set_uniform_float("far_plane", far_plane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);
    renderQuad();

    if (use_im_gui) {
      ImGui::Begin("Link options");
      ImGui::Checkbox("Shadow", &use_shadow);
      ImGui::Checkbox("Counter with wireFrame", &contour_wireframe);
      ImGui::Checkbox("Use Zatoon", &use_zAtoon);
      ImGui::Checkbox("Vertex shifted along normal for contour", &vertex_shifted_along_normal);
      ImGui::SliderFloat("Rotation", &rotation, 0.0f, 720.0f);
      ImGui::SliderFloat("Displacement contour", &displacement, 0.0f, 0.01f);
      ImGui::SliderFloat("Light diffuse", &light_diffuse, 0.0f, 1.0f);
      ImGui::SliderFloat("Light ambient", &light_ambient, 0.0f, 1.0f);
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

}
}