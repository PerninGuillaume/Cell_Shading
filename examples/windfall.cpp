#include "windfall.h"

#include "../Camera.h"
#include <iostream>

#include "../stb_image.h"
#include "../Model.h"

namespace windfall {

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastXMouse = 800 / 2;
float lastYMouse = 600 / 2;
float alpha_clip = 0.3f;
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

  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
  {
    alpha_clip = std::min(1.0f, alpha_clip += 0.01f);
    std::cout << "alpha_clip = " << alpha_clip << std::endl;
  }

  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
    alpha_clip = std::max(0.0f, alpha_clip -= 0.01f);
    std::cout << "alpha_clip = " << alpha_clip << std::endl;
  }
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


unsigned int loadCubemap(const std::vector<std::string>& faces)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++)
  {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
      );
      stbi_image_free(data);
    }
    else
    {
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

  //Capture the mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  program *program_windfall = init_program(window, "shaders/vertex_model.glsl",
                                  "shaders/fragment_model.glsl");
  program *program_skybox = init_program(window, "shaders/vertex_skybox.glsl",
                                         "shaders/fragment_skybox.glsl");
  program *program_water = init_program(window, "shaders/vertex_water.glsl",
                                        "shaders/fragment_water.glsl");
  //stbi_set_flip_vertically_on_load(true);
  Model windfall("models/Windfall Island/Windfall/Windfall.obj");

  glEnable(GL_DEPTH_TEST);
  //Skybox

  std::vector<std::string> faces = {
      "images/skybox/right.jpg",
      "images/skybox/left.jpg",
      "images/skybox/top.jpg",
      "images/skybox/bottom.jpg",
      "images/skybox/front.jpg",
      "images/skybox/back.jpg"
  };
  unsigned int cubemapTexture = loadCubemap(faces);
  program_skybox->set_uniform_int("skybox", 0);

  float skyboxVertices[] = {
      // positions
      -1.0f,  1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f, -1.0f,  1.0f,
      -1.0f, -1.0f,  1.0f,

      -1.0f,  1.0f, -1.0f,
      1.0f,  1.0f, -1.0f,
      1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f,
      -1.0f,  1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,
      1.0f, -1.0f,  1.0f
  };

  unsigned int skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  float heightf = -10.0;
  float waterVertices[] = {
          -20000.0f,  heightf, -20000.0f,
          20000.0f,  heightf, -20000.0f,
          20000.0f,  heightf,  20000.0f,
          20000.0f,  heightf,  20000.0f,
          -20000.0f,  heightf,  20000.0f,
          -20000.0f,  heightf, -20000.0f,
  };

  unsigned int waterVAO, waterVBO;
  glGenVertexArrays(1, &waterVAO);
  glGenBuffers(1, &waterVBO);
  glBindVertexArray(waterVAO);
  glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), &waterVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  /*
//DirLight
  program_windfall->set_uniform_vec3("dirLight.direction", -0.3f, -0.7f, -0.3f);
  program_windfall->set_uniform_vec3("dirLight.ambient",  2.0f);
  program_windfall->set_uniform_vec3("dirLight.specular", 0.2f);
  auto diffuseColor = glm::vec3(1.0f);
  program_windfall->set_uniform_vec3("dirLight.diffuse", diffuseColor); // darken diffuse light a bit
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

  program_windfall->set_uniform_vector_float("zAtoon", 256, zAtoon_data);
   */
  double lastTime = glfwGetTime();
  int nbFrames = 0;
  while (!glfwWindowShouldClose(window)) {

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    deltaTime_since_last_press = currentFrame - time_of_last_press;
    lastFrame = currentFrame;

    nbFrames++;
    if ( currentFrame - lastTime >= 1.0 ){
      std::cout << 1000.0 / nbFrames << " ms/frame" << std::endl;
      nbFrames = 0;
      lastTime += 1.0;
    }
    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera.fov_camera), 800.0f / 600.0f,
                                            0.1f, 100.0f);

    //Skybox rendering
    //glDepthMask(GL_FALSE);
    program_skybox->use();

    glDepthFunc(GL_LEQUAL);
    glm::mat4 view = glm::mat4(glm::mat3(camera.view_matrix())); // Remove the translation from the view matrix
    program_skybox->set_uniform_mat4("view", view);
    program_skybox->set_uniform_mat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    //glDepthMask(GL_TRUE);

    // Water rendering

    program_water->use();
    view = camera.view_matrix();
    glm::mat4 model = glm::mat4(1.0f);
    program_water->set_uniform_mat4("view", view);
    program_water->set_uniform_mat4("projection", projection);
    program_water->set_uniform_mat4("model", model);

    glBindVertexArray(waterVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);




    //Windfall rendering

    view = camera.view_matrix();

    program_windfall->set_uniform_mat4("view", view);
    program_windfall->set_uniform_mat4("projection", projection);
    program_windfall->set_uniform_float("alpha_clip", alpha_clip);

    //program_windfall->set_uniform_vec3("viewPosition", camera.position);
    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, glm::vec3(1.0f, -10.0f, -25.0f));
    model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
    program_windfall->set_uniform_mat4("model", model);

    windfall.draw(program_windfall);


    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glfwTerminate();

}
}

