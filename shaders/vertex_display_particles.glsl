#version 450

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VTexCoord;

out vec2 TexCoord;


uniform mat4 MVP;

void main()
{
    TexCoord = VTexCoord;

    gl_Position = MVP * vec4(VertexPosition,1.0);
}
