#version 450

in vec3 vertexColor;
in vec3 normal_unit;
in vec3 light_dir;

out vec4 ouput_color;

void main() {
    ouput_color = vec4(
        clamp((dot(normal_unit, light_dir)) * vertexColor, 0.0, 1.0), 1.0);
}
