#pragma once

#include "program.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Particles {
private:
    Particles();

    program *render3DProg;
    program *renderParticlesProg;
    program *computeProg;

    // Texture rendering
    GLuint renderedTexture;
    GLuint renderFramebuffer;

    // Particles
    glm::ivec3 nParticles;
    GLuint totalParticles;
    GLuint particlesVao;
    GLuint bhVao, bhBuf;  // black hole VAO and buffer
    glm::vec4 bh1, bh2;


    // Display
    GLuint dataBuf, fsQuad;

    glm::vec2 center;
    float cheight;

    glm::mat4 m_projection;

    int width, height;
    float time, deltaT, speed;
    float angle, rotSpeed;
    float angleParticles;

    void initBuffers();
    void setMatrices(const glm::mat4& view, const glm::mat4& model, const glm::mat4& projection);


public:
    Particles(program *program_compute, program *program_render, program *program_display);

    void init();
    void update( float t );
    int render(glm::mat4 projections);
    void resize(int, int);
};