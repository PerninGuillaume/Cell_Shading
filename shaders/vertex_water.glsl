#version 450

layout (location = 0) in vec3 aPos;


out vec2 TexCoords;
out float distToCamera;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 cs_position = view * model * vec4(aPos, 1.0);
    distToCamera = -cs_position.z;
    gl_Position = projection * cs_position;
}