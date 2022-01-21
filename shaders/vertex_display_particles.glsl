#version 450

layout (location = 0) in vec3 VertexPosition;
//layout (location = 1) in vec3 VertexNormal;
layout (location = 1) in vec2 VTexCoord;

out vec2 TexCoord;


uniform mat4 MVP;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_up = vec3(0.f, 1.f, 0.f);
uniform vec3 camera_right = vec3(1.f, 0.f, 0.f);
uniform mat4 model;
uniform vec3 billboard_size_ = vec3(0.3,0.5,1.0);


void main()
{
    TexCoord = VTexCoord;
    vec3 aPos = VertexPosition;

    vec3 vertexPosition = camera_right * aPos.x * billboard_size_.x
     + camera_up * aPos.y * billboard_size_.y;

    vertexPosition.y += 2.0f * billboard_size_.y;

    gl_Position = projection * view * model * vec4(vertexPosition, 1.0f);
}