#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_up;
uniform vec3 camera_right;
uniform vec3 wave_center;
uniform vec3 billboard_size;
uniform vec3 center_waves;
uniform float dispacement_percentage;

void main()
{
    TexCoords = aTexCoords;

    //gl_Position = projection * view * model * vec4(aPos, 1.0);
    bool billboard = true;
    if (billboard) {
        vec3 direction_wave = normalize(wave_center - center_waves) * 10.0f;

        vec3 vertexPosition = wave_center
        + camera_right * aPos.x * billboard_size.x
        + camera_up * aPos.y * billboard_size.y;

        vertexPosition.y += 2.0f * billboard_size.y;
        vertexPosition -= direction_wave * dispacement_percentage;

        gl_Position = projection * view * vec4(vertexPosition, 1.0f);
    }

}