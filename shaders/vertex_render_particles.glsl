#version 450

layout (location = 0) in vec4 VertexPosition;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * VertexPosition;
}
