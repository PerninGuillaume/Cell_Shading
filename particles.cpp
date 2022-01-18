#include "particles.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


using std::endl;
using std::cerr;
using std::vector;
using glm::vec3;

Particles::Particles(): nParticles(100,100,100),
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
//  projection = glm::perspective( glm::radians(50.0f), (float)width/height, 1.0f, 100.0f);
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
  glm::vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
  GLfloat dx = 2.0f / (nParticles.x - 1),
          dy = 2.0f / (nParticles.y - 1),
          dz = 2.0f / (nParticles.z - 1);
  // We want to center the particles at (0,0,0)
  glm::mat4 transf = glm::translate(glm::mat4(1.0f), glm::vec3(-1,-1,-1));
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
      }
    }
  }

  // We need buffers for position , and velocity.
  GLuint bufs[2];
  glGenBuffers(2, bufs);
  GLuint posBuf = bufs[0];
  GLuint velBuf = bufs[1];

  GLuint bufSize = totalParticles * 4 * sizeof(GLfloat);

  // The buffers for positions
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initPos[0], GL_DYNAMIC_DRAW);

  // Velocities
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velBuf);
  glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initVel[0], GL_DYNAMIC_COPY);

  // Set up the VAO
  glGenVertexArrays(1, &particlesVao);
  glBindVertexArray(particlesVao);

  glBindBuffer(GL_ARRAY_BUFFER, posBuf);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);

  // Set up a buffer and a VAO for drawing the attractors (the "black holes")
  glGenBuffers(1, &bhBuf);
  glBindBuffer(GL_ARRAY_BUFFER, bhBuf);
  GLfloat data[] = { bh1.x, bh1.y, bh1.z, bh1.w, bh2.x, bh2.y, bh2.z, bh2.w };
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), data, GL_DYNAMIC_DRAW);

  glGenVertexArrays(1, &bhVao);
  glBindVertexArray(bhVao);

  glBindBuffer(GL_ARRAY_BUFFER, bhBuf);
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

void Particles::render(glm::mat4 projection)
{

  // Save viewport
  GLint aiViewport[4];
  glGetIntegerv(GL_VIEWPORT, aiViewport);

  //////////////////////////
  // RENDER TO FRAMEBUFFER
  /////////////////////////

  glBindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);
  glViewport(0,0,256,256); // Render on the whole framebuffer, complete from the lower left corner to the upper right

  //////////////////////////
  // PARTICLES RENDERING
  //////////////////////////

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1,1,1,0);


  // Rotate
  glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(angleParticles), glm::vec3(0,0,1));
  glm::vec3 att1 = glm::vec3(rot*bh1);
  glm::vec3 att2 = glm::vec3(rot*bh2);

  // Execute the compute shader
  computeProg->use();
  computeProg->set_uniform_vec3("BlackHolePos1", att1);
  computeProg->set_uniform_vec3("BlackHolePos2", att2);
  glDispatchCompute(totalParticles / 1000, 1, 1);
  glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT );


  renderParticlesProg->use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glm::mat4  view = glm::lookAt(glm::vec3(2,0,20), glm::vec3(0,0,0), glm::vec3(0,1,0));
  glm::mat4 model = glm::mat4(1.0f);
  setMatrices(projection, view, model);

  // particles
  glPointSize(1.0f);

  renderParticlesProg->set_uniform_vec4("Color", glm::vec4(0.8f,0.1f,0.1f,0.5f));
  glBindVertexArray(particlesVao);
  glDrawArrays(GL_POINTS,0, totalParticles);
  glBindVertexArray(0);

  // attractor
  glPointSize(5.0f);
  GLfloat data[] = { att1.x, att1.y, att1.z, 1.0f, att2.x, att2.y, att2.z, 1.0f };
  glBindBuffer(GL_ARRAY_BUFFER, bhBuf);
  glBufferSubData( GL_ARRAY_BUFFER, 0, 8 * sizeof(GLfloat), data );
  renderParticlesProg->set_uniform_vec4("Color", glm::vec4(1,1,0,1.0f));
  glBindVertexArray(bhVao);
  glDrawArrays(GL_POINTS, 0, 2);
  glBindVertexArray(0);


//  glClearColor(1,1,1,1);

  /////////////////////////
  // TEXTURE RENDERING
  /////////////////////////


  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(aiViewport[0], aiViewport[1], (GLsizei)aiViewport[2], (GLsizei)aiViewport[3]);


  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  render3DProg->use();

  view = glm::lookAt( glm::vec3(0,0,2), glm::vec3(0,0,0), glm::vec3(0,1,0) );
  model = glm::rotate( glm::mat4(1.0f), glm::radians(angle), glm::vec3(1,1.5f,0.5f));
  glm::mat4 mv = view * model;
  glm::mat3 norm = glm::mat3( vec3(mv[0]), vec3(mv[1]), vec3(mv[2]) );
  glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)width/height, 1.0f, 100.0f);

  render3DProg->set_uniform_mat4("MVP", proj * mv);

//  TEXTURE.render();
}

void Particles::setMatrices(glm::mat4 view, glm::mat4 model, glm::mat4 projection) {
  glm::mat4 mv = view * model;
  glm::mat3 norm = glm::mat3( vec3(mv[0]), vec3(mv[1]), vec3(mv[2]) );

  renderParticlesProg->set_uniform_mat4("MVP", projection * mv);
}

void Particles::resize(int w, int h)
{
  glViewport(0,0,w,h);
  width = w;
  height = h;
}

Particles::Particles(program *program_compute, program *program_render, program *program_display) :
  Particles()
{
  render3DProg = program_display;
  renderParticlesProg = program_render;
  computeProg = program_compute;
  init();
}
