#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texture_coordinates;

out vec3 vertexColor;
out vec2 TexCoord;

uniform mat4 transform;

void main() {
    gl_Position = transform * vec4(vPosition, 1.0f);
    TexCoord = texture_coordinates;
    vertexColor = color;

}
