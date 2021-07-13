#include "link.h"
#include "../stb_image.h"
#include "../Model.h"
#include "../Camera.h"
#include "../Shadow.h"
#include "../misc.h"
#include "../callback.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "../Helper.h"

namespace link {

bool use_im_gui = true;
float lastFrame = 0.0f;

std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

void display(GLFWwindow *window) {

  int width, height;
  glfwGetWindowSize(window, &width, &height);
  unsigned int SRC_WIDTH = width;
  unsigned int SRC_HEIGHT = height;

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

  program *shader = init_program("shaders/vertex_link.glsl",
                                      "shaders/fragment_link.glsl");
  program *programOutline = init_program("shaders/vertex_scale_up.glsl",
                                         "shaders/fragment_black.glsl");
  program *shadow_shader_depth = init_program("shaders/vertex_shadow_depth.glsl",
                                              "shaders/fragment_shadow_depth.glsl");

  program *quad_depth_shader = init_program("shaders/vertex_normalized_coord.glsl", "shaders/fragment_quad_depth.glsl");
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
  bool pcf = false;
  bool contour_wireframe= false;
  bool vertex_shifted_along_normal = false;
  float rotation = 0;
  float displacement = 0.006f;
  float light_diffuse = 1.0f;
  float light_ambient = 0.5f;
  Helper helper = Helper(camera, use_im_gui);

  while (!glfwWindowShouldClose(window)) {
    if (use_im_gui) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
    }

    float currentFrame = glfwGetTime();
    helper.deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwSetWindowUserPointer(window, &helper); //For callbacks, set the pointer to use
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



    glViewport(0, 0, shadow.shadow_width, shadow.shadow_height);
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
    glm::mat4 projection = glm::perspective(glm::radians(camera->fov_camera), (float)SRC_WIDTH / (float)SRC_HEIGHT,
                                            0.1f, 100.0f);
    glm::mat4 view = camera->view_matrix();

    for (auto prg : programs)
    {
        prg->set_uniform_mat4("view", view);
        prg->set_uniform_mat4("projection", projection);
    }


    shader->set_uniform_mat4("model", model);
    shader->set_uniform_bool("use_zAtoon", use_zAtoon);
    shader->set_uniform_bool("use_shadow", use_shadow);
    shader->set_uniform_bool("pcf", pcf);
    shader->set_uniform_vec3("dirLight.ambient",  light_ambient);
    shader->set_uniform_vec3("dirLight.diffuse", light_diffuse); // darken diffuse light a bit
    //shader->set_uniform_vec3("viewPos", camera->position);
    shader->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);



    std::vector<glm::vec3> offset_vecs{camera->right, -camera->right, camera->up, -camera->up};

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
    //renderQuad();

    if (use_im_gui) {
      ImGui::Begin("Link options");
      ImGui::Checkbox("Shadow", &use_shadow);
      ImGui::Checkbox("PCF", &pcf);
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