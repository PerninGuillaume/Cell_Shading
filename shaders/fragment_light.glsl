#version 450

out vec4 FragColor;

uniform vec3 light;

void main() {
    FragColor = vec4(light, 1.0f);
}
