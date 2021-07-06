#version 450

in vec3 vertexColor;
out vec4 ouput_color;

void main() {
    ouput_color = vec4(vertexColor, 1.0);
}
