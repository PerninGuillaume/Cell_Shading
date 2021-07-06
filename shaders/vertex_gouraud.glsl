#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;

out vec3 vertexColor;

uniform vec3 color_uniform;
uniform mat4 projection_matrix;
uniform vec3 light_position;
uniform float light_intensity;

vec3 normal_unit;
vec3 light_dir;

void main() {
    gl_Position = vec4(vPosition, 1.0f);
    normal_unit = normalize(normal);
    light_dir = normalize(light_position - vPosition);
    vertexColor = clamp((dot(normal_unit, light_dir)) * color * light_intensity, 0, 1);
    //TODO why the triangle isn't in rainbow color

    //vertexColor = vec4(normal_unit, 1);
    //vertexColor = color_uniform;
    //vertexColor = color; //Get the color from the vertex buffer object
    //gl_Position = projection_matrix * gl_Position;
}
