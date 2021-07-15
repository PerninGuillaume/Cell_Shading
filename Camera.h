#pragma once

#include "program.h"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    RIGHT,
    LEFT
};

// This class was created following the tutorial of www.learnopengl.com, it was extended afterwards for our needs
class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 up);

    void processKeyboard(Camera_Movement direction, float deltaTime);
    void processMouse(float xoffset, float yoffset);
    void processScroll(float yoffset);
    glm::mat4 view_matrix() const;
    void shift_pressed(bool is_pressed);
  void ctrl_pressed(bool is_pressed);

  glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    const glm::vec3 worldUp; //Original up value, needed
    glm::vec3 right;

    static constexpr float movement_speed = 2.5f;
    static constexpr float mouse_sensitivity = 0.1f;
    static constexpr float max_fov = 45.0f;

    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov_camera = 45.0f;
    bool shiftPressed = false;
    bool ctrlPressed = false;
    float sprintSpeedUp = 10.0f;
    float crouchSlowDown = 0.1f;

    void refresh_vectors();
};


