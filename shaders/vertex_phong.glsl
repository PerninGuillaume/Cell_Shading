#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;


uniform vec3 color_uniform;
uniform vec3 light_position;
uniform mat4 transform;

out vec3 vertexColor;
out vec3 normal_unit;
out vec3 light_dir;

void main() {
    //vertexColor = color; //Get the color from the vertex buffer object
    gl_Position = transform * vec4(vPosition, 1.0f);
    light_dir = normalize(light_position - vPosition);
    normal_unit = normalize(normal);

    vertexColor = color;

    //vertexColor = vec4(normal_unit, 1);
    //vertexColor = color_uniform;
    //gl_Position = projection_matrix * gl_Position;
}
