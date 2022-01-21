#include "particles.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


using std::endl;
using std::cerr;
using std::vector;
using glm::vec3;

Particles::Particles(): nParticles(20,50,20),
                                    bh1(5,0,0,1), bh2(-5,0,0,1),
                                    dataBuf(0), fsQuad(0),
                                    center(0.001643721971153f, 0.822467633298876f), cheight(2.0f),
                                    time(0.0f), deltaT(0.0f), speed(200.0f), angle(0.0f), rotSpeed(60.0f), angleParticles(0.0f)
{
  totalParticles = nParticles.x * nParticles.y * nParticles.z;
}

void Particles::init()
{
  initBuffers();
  m_projection = glm::perspective( glm::radians(50.0f), (float)800/600, 1.0f, 100.0f);
//  glEnable(GL_DEPTH_TEST);
}

void Particles::initBuffers()
{
//////////////////////
  // PARTICLE STUFF  ///
  //////////////////////

  // Initial positions of the particles
  vector<GLfloat> initPos;
  vector<GLfloat> initVel(totalParticles * 4, 0.0f);
  vector<GLfloat> initLife;

  glm::vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
  GLfloat dx = 2.0f / (nParticles.x - 1) * 8,
          dy = 2.0f / (nParticles.y - 1) * 3,
          dz = 2.0f / (nParticles.z - 1) * 8;
  // We want to center the particles at (0,0,0)
  float max_z = 0;
  float max_x = 0;
  float min_z = 0;
  float min_x = 0;
  glm::mat4 transf = glm::translate(glm::mat4(1.0f), glm::vec3(-8,-5,-8));
  for( int i = 0; i < nParticles.x; i++ ) {
    for( int j = 0; j < nParticles.y; j++ ) {
      for( int k = 0; k < nParticles.z; k++ ) {
        p.x = dx * i;
        p.y = dy * j;
        p.z = dz * k;
        p.w = 1.0f;
        p = transf * p;
        initPos.push_back(p.x);
        initPos.push_back(p.y);
        initPos.push_back(p.z);
        initPos.push_back(p.w);

        if (p.x > max_x)
          max_x = p.x;
        if (p.x < min_x)
          min_x = p.x;
        if (p.z < min_z)
          min_z = p.z;
        if (p.z > max_z)
          max_z = p.z;
        float life = sqrt(50 - p.x * p.x - p.z * p.z * 1) / 0.8f;
        initLife.push_back(life);
        initLife.push_back(0.0f);
        initLife.push_back(0.0f);
        initLife.push_back(0.0f);
      }
    }
  }

  std::cout << "X(" << min_x << " : " << max_x << ") / Z(" << min_z << " : " << max_z << ")\n";


  // We need buffers for position , and velocity.
  GLuint bufs[5];
  glGenBuffers(5, bufs);
  GLuint posBuf = bufs[0];
  GLuint velBuf = bufs[1];
  GLuint initPosBuf = bufs[2];
  GLuint initLifeBuf = bufs[3];
  GLuint currentLifeBuf = bufs[4];

  GLuint bufSize = totalParticles * 4 * sizeof(GLfloat);

  // The buffers for positions
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initPos[0], GL_DYNAMIC_DRAW);

  // Velocities
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initVel[0], GL_DYNAMIC_COPY);

  // Initial positions
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, initPosBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initPos[0], GL_DYNAMIC_COPY);

  // Life
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, initLifeBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initLife[0], GL_DYNAMIC_COPY);

  // Current Life
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, currentLifeBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initLife[0], GL_DYNAMIC_COPY);

  // Set up the VAO
  glGenVertexArrays(1, &particlesVao);
  glBindVertexArray(particlesVao);

  glBindBuffer(GL_ARRAY_BUFFER, posBuf);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);

  /////////////////////////
  // FRAME BUFFER STUFF ///
  /////////////////////////

  glGenFramebuffers(1, &renderFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);

  glGenTextures(1, &renderedTexture);

  glBindTexture(GL_TEXTURE_2D, renderedTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256,256, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  GLuint depthrenderbuffer;
  glGenRenderbuffers(1, &depthrenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 256,256);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

  // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)

    glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderedTexture);

  glBindImageTexture(0, renderedTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
}

void Particles::update( float t )
{
  if( time == 0.0f ) {
    deltaT = 0.0f;
  } else {
    deltaT = t - time;
  }
  time = t;

  angle = 45;
  angleParticles += 35.0f * deltaT;
  if( angleParticles > 360.0f ) angleParticles -= 360.0f;
}

int Particles::render(glm::mat4 projection)
{
  // Save viewport
  GLint aiViewport[4];
  glGetIntegerv(GL_VIEWPORT, aiViewport);

  //////////////////////////
  // RENDER TO FRAMEBUFFER
  /////////////////////////

   glBindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);
//  glBindFramebuffer(GL_FRAMEBUFFER, 0);

   glViewport(0,0,256,256); // Render on the whole framebuffer, complete from the lower left corner to the upper right

  //////////////////////////
  // PARTICLES RENDERING
  //////////////////////////

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0,0,0,0);

  // Execute the compute shader
  computeProg->use();
  glDispatchCompute(totalParticles / 1000, 1, 1);
  glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );

  // Draw the scene
  renderParticlesProg->use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::mat4 view = glm::lookAt(glm::vec3(2,0,20), glm::vec3(0,0,0), glm::vec3(0,1,0));
  glm::mat4 model = glm::mat4(1.0f);
  setMatrices(view, model, m_projection);

  // Draw the particles
  glPointSize(3.0f);
  renderParticlesProg->set_uniform_vec4("Color", glm::vec4(0.8f,0.1f,0.1f,0.8f));
  glBindVertexArray(particlesVao);
  glDrawArrays(GL_POINTS,0, totalParticles);
  glBindVertexArray(0);


  // /////////////////////////
  // // Texture RENDERING
  // /////////////////////////

   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   glViewport(aiViewport[0], aiViewport[1], (GLsizei)aiViewport[2], (GLsizei)aiViewport[3]);

   return renderedTexture;
}
#include <glm/gtx/string_cast.hpp>

void Particles::setMatrices(const glm::mat4& view, const glm::mat4& model, const glm::mat4& projection) {
  glm::mat4 mv = view * model;
  glm::mat3 norm = glm::mat3( vec3(mv[0]), vec3(mv[1]), vec3(mv[2]) );

//  std::cout << glm::to_string(projection * mv) << std::endl;
  renderParticlesProg->set_uniform_mat4("ModelViewMatrix", mv);
  renderParticlesProg->set_uniform_mat4("MVP", projection * mv);
}

Particles::Particles(program *program_compute, program *program_render, program *program_display) :
  Particles()
{
  render3DProg = program_display;
  renderParticlesProg = program_render;
  computeProg = program_compute;
  init();
}
