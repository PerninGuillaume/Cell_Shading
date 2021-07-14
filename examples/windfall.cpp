#include "windfall.h"

#include <iostream>
#include <memory>

#include "../Camera.h"
#include "../callback.h"
#include "../Skybox.h"
#include "../Helper.h"
#include "../Model.h"
#include "../misc.h"
#include "../Shadow.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace windfall {

bool use_im_gui = true;
float lastFrame = 0.0f;

std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

unsigned int water_create_VAO() {
  float heightf = -10.05;
  float waterVertices[] = {
      -20000.0f, heightf, -20000.0f, 20000.0f, heightf, -20000.0f, 20000.0f, heightf, 20000.0f, 20000.0f, heightf, 20000.0f, -20000.0f, heightf, 20000.0f, -20000.0f, heightf, -20000.0f,};

  unsigned int waterVAO, waterVBO;
  glGenVertexArrays(1, &waterVAO);
  glGenBuffers(1, &waterVBO);
  glBindVertexArray(waterVAO);
  glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), &waterVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

  return waterVAO;
}

void display(GLFWwindow *window) {
  int SRC_WIDTH, SRC_HEIGHT;
  glfwGetWindowSize(window, &SRC_WIDTH, &SRC_HEIGHT);
  //Capture the mouse
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

  program *shadow_shader_depth = init_program("shaders/vertex_shadow_depth.glsl",
                                              "shaders/fragment_shadow_depth.glsl");
  program *quad_depth_shader = init_program("shaders/vertex_normalized_coord.glsl", "shaders/fragment_quad_depth.glsl");
  program *shader_normals = init_program("shaders/vertex_normals.glsl", "shaders/fragment_normals.glsl",
                                         "shaders/geometry_normals.glsl");
  program *program_windfall;
  program *program_windfall_without_lighting = init_program("shaders/vertex_model.glsl",
                                  "shaders/fragment_model.glsl");
  program *program_windfall_with_lighting = init_program("shaders/vertex_link.glsl",
                                           "shaders/fragment_link.glsl");
  program_windfall = program_windfall_with_lighting;
  program *program_skybox = init_program("shaders/vertex_skybox.glsl",
                                         "shaders/fragment_skybox.glsl");
  program *program_water = init_program("shaders/vertex_water.glsl",
                                        "shaders/fragment_water.glsl");
  program *program_clouds = init_program("shaders/vertex_clouds.glsl",
                                         "shaders/fragment_clouds.glsl");
  //stbi_set_flip_vertically_on_load(true);
  Model windfall_flat("models/Windfall Island/Windfall/Windfall_save.obj");
  Model windfall_smooth("models/Windfall Island/Windfall/Windfall.obj");

  unsigned int size_shadow_texture = 4096;
  Shadow shadow = Shadow(size_shadow_texture, size_shadow_texture);

  glEnable(GL_DEPTH_TEST);

  //Skybox

  unsigned int cubemapTexture = loadSkyBox(program_skybox);
  unsigned int skyboxVAO = skyBox_create_VAO();

  // Water

  unsigned int waterVAO = water_create_VAO();
  set_zAtoon(program_windfall);


  //Setup of different default values
  bool with_lighting = true;
  bool wireframe = false;
  bool use_zAtoon = false;
  bool no_texture = false;
  bool display_normals = false;
  bool flat_look = false;
  bool use_shadow = true;
  bool display_depth_map = false;
  bool peter_paning = false;
  bool pcf = true;
  float shadow_bias = 0.005f;
  float near_plane_light = 20.0f, far_plane_light = 90.0f;
  float light_ambient = 0.7f;
  float light_diffuse = 0.8f;
  float light_dir[3] = {-0.3f, -0.7f, -0.3f};
  float light_pos[3] = {-21.0f, 49.0f, -29.0f}; //need a position for shadow
  float light_shadow_center[3] = {6.0f, 1.0f, -44.0f}; //The point the light will look at
  ImVec4 some_color = ImVec4(0.45f, 0.55f, 0.6f, 1.00f);
  float alpha_clip = 0.3f;
  float offset = 0.0f;
  float ortho_bounds[4] = {-50.0f, 50.0f, -60.0f, 70.0f};
  bool ortho_view = false;
  Helper helper = Helper(camera, use_im_gui);


  // Clouds
  std::vector<unsigned int> cloudsTextures = loadClouds();
  unsigned int cloudsVAO = clouds_create_VAO();
  while (!glfwWindowShouldClose(window)) {

    if (use_im_gui) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
    }

    float currentFrame = glfwGetTime();
    helper.deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwSetWindowUserPointer(window, &helper);
    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera->view_matrix();
    glm::mat4 projection;
    if (!ortho_view) {
      projection = glm::perspective(glm::radians(camera->fov_camera), 800.0f / 600.0f,
                                              0.1f, 1000.0f);
    } else {
      projection = glm::ortho(ortho_bounds[0], ortho_bounds[1], ortho_bounds[2], ortho_bounds[3], near_plane_light, far_plane_light);
    }




    //--------------------Windfall rendering-----------------------

    if (wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (with_lighting) {
      program_windfall = program_windfall_with_lighting;
    } else {
      program_windfall = program_windfall_without_lighting;
    }


    // 1. Render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    //lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane_light, far_plane_light);
    lightProjection = glm::ortho(ortho_bounds[0], ortho_bounds[1], ortho_bounds[2], ortho_bounds[3], near_plane_light, far_plane_light);
    glm::vec3 eye = glm::vec3(light_pos[0], light_pos[1], light_pos[2]);
    glm::vec3 center = glm::vec3(light_shadow_center[0], light_shadow_center[1], light_shadow_center[2]);
    glm::vec3 up = glm::vec3(0.6f, 0.02f, -0.77f);
    //glm::vec3 center = glm::vec3(0.0f);
    lightView = glm::lookAt(eye,
                            center,
                            up);
    lightSpaceMatrix = lightProjection * lightView;

    glViewport(0, 0, shadow.shadow_width, shadow.shadow_height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 model_mat_windfall = glm::mat4(1.0f);
    model_mat_windfall = glm::translate(model_mat_windfall, glm::vec3(1.0f, -10.0f, -25.0f));
    model_mat_windfall = glm::scale(model_mat_windfall, glm::vec3(0.01f, 0.01f, 0.01f));


    shadow_shader_depth->set_uniform_mat4("model", model_mat_windfall);
    shadow_shader_depth->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);

    if (peter_paning) {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
    }
    windfall_smooth.draw(shadow_shader_depth); //The choice of the normal has no influence on the depth
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // reset viewport

    glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. render scene as normal using the generated depth/shadow map
    // --------------------------------------------------------------


    program_windfall->set_uniform_mat4("model", model_mat_windfall);
    program_windfall->set_uniform_mat4("view", view);
    program_windfall->set_uniform_mat4("projection", projection);
    program_windfall->set_uniform_float("alpha_clip", alpha_clip);
    program_windfall->set_uniform_bool("use_zAtoon", use_zAtoon);
    program_windfall->set_uniform_bool("no_texture", no_texture);
    program_windfall->set_uniform_bool("use_shadow", use_shadow);
    program_windfall->set_uniform_bool("pcf", pcf);
    program_windfall->set_uniform_float("shadow_bias", shadow_bias);

    program_windfall->set_uniform_vec3("dirLight.direction", light_dir[0], light_dir[1], light_dir[2]);

    program_windfall->set_uniform_vec3("dirLight.ambient", light_ambient);
    program_windfall->set_uniform_vec3("dirLight.diffuse", light_diffuse);

    program_windfall->set_uniform_int("shadowMap", 1);
    program_windfall->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);

    if (flat_look)
      windfall_flat.draw(program_windfall);
    else
      windfall_smooth.draw(program_windfall);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

//------------------ Water rendering-----------------------------

    program_water->use();
    glm::mat4 model_mat_water = glm::mat4(1.0f);
    program_water->set_uniform_mat4("view", view);
    program_water->set_uniform_mat4("projection", projection);
    program_water->set_uniform_mat4("model", model_mat_water);


    glBindVertexArray(waterVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);




//-----------------------Normals------------------------------------
    if (display_normals) {
      shader_normals->set_uniform_mat4("projection", projection);
      shader_normals->set_uniform_mat4("view", view);
      shader_normals->set_uniform_mat4("model", model_mat_windfall);
      if (flat_look)
        windfall_flat.draw(shader_normals);
      else
        windfall_smooth.draw(shader_normals);
    }

//--------------------------Skybox rendering---------------------------
    program_skybox->use();

    glDepthFunc(GL_LEQUAL);
    view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
    program_skybox->set_uniform_mat4("view", view);
    program_skybox->set_uniform_mat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    //------------------ Clouds rendering --------------------------

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program_clouds->use();

    program_clouds->set_uniform_int("tex_cloud", 0);
    program_clouds->set_uniform_int("tex_cloud_mask", 1);
    view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
//    view = camera->view_matrix();
    if (offset > 1.0)
      offset = -1.0;
    program_clouds->set_uniform_float("offset", offset);

    glm::mat4 model_mat_clouds = glm::mat4(1.0f);
//    view = glm::translate(view, glm::vec3(-offset * 10));
    program_clouds->set_uniform_mat4("view", view);
    program_clouds->set_uniform_mat4("projection", projection);
    program_clouds->set_uniform_float("alpha_clip", alpha_clip);
    program_clouds->set_uniform_mat4("model", model_mat_clouds);
    program_clouds->set_uniform_vec3("camera_right", camera->right);
    program_clouds->set_uniform_vec3("camera_up", camera->up);
    offset += helper.deltaTime / 100;
//    std::cout <<


    glBindVertexArray(cloudsVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cloudsTextures[2]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudsTextures[3]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * 4);
    glBindVertexArray(0);


    if (display_depth_map) {

      quad_depth_shader->use();
      quad_depth_shader->set_uniform_int("depthMap", 0);
      quad_depth_shader->set_uniform_float("near_plane", near_plane_light);
      quad_depth_shader->set_uniform_float("far_plane", far_plane_light);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);
      renderQuad();
    }

    //std::cout << "Camera Position : " << camera->position.x << ' ' << camera->position.y << ' ' << camera->position.z << std::endl;
    //std::cout << "Camera up : " << camera->up.x << ' ' << camera->up.y << ' ' << camera->up.z << std::endl;

    if (use_im_gui) {
      ImGui::Begin("Windfall options");
      ImGui::SliderFloat4("Ortho bounds", ortho_bounds, -100.0f, 100.0f);
      ImGui::Checkbox("Ortho view", &ortho_view);
      ImGui::Checkbox("WireFrame", &wireframe);
      ImGui::Checkbox("Shadow", &use_shadow);
      ImGui::Checkbox("Depth texture", &display_depth_map);
      ImGui::Checkbox("Peter Paning", &peter_paning);
      ImGui::Checkbox("PCF", &pcf);
      ImGui::InputFloat("Shadow bias", &shadow_bias, 0.0001f, 0.01f);
      //ImGui::SliderFloat("Shadow bias", &shadow_bias, 0.0f, 0.01f);
      ImGui::SliderFloat("Near plane light frustrum", &near_plane_light, 1.0f, 200.0f);
      ImGui::SliderFloat("Far plane light frustrum", &far_plane_light, 0.0f, 400.0f);
      ImGui::Checkbox("Use Zatoon", &use_zAtoon);
      ImGui::Checkbox("No texture", &no_texture);
      ImGui::Checkbox("Enable lighting", &with_lighting);
      ImGui::Checkbox("Display Normals", &display_normals);
      ImGui::Checkbox("Flat look", &flat_look);
      ImGui::SliderFloat("Alpha clip", &alpha_clip, 0.0f, 1.0f);
      ImGui::SliderFloat("Light diffuse", &light_diffuse, 0.0f, 1.0f);
      ImGui::SliderFloat("Light ambient", &light_ambient, 0.0f, 1.0f);
      ImGui::SliderFloat3("Light position", light_pos, -100.0f, 100.0f);
      ImGui::SliderFloat3("Light shadow center", light_shadow_center, -100.0f, 100.0f);
      ImGui::ColorEdit3("Some color", (float*)&some_color);
      ImGui::SliderFloat3("Light direction", light_dir, -1.0f, 1.0f);
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

