#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in float apex_percentage;
layout (location = 3) in vec3 wave_center;
layout (location = 4) in vec3 billboard_size;
layout (location = 5) in float displacement_percentage;

out vec2 TexCoords;
out float apex_percentage_frag;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_up = vec3(0.f, 1.f, 0.f);
uniform vec3 camera_right = vec3(1.f, 0.f, 0.f);
uniform vec3 center_waves;

void main()
{
    TexCoords = aTexCoords;

    vec3 direction_wave = normalize(wave_center - center_waves) * 10.0f;

    vec3 billboard_size_ = billboard_size;
    vec3 vertexPosition = wave_center
    + camera_right * aPos.x * billboard_size_.x
    + camera_up * aPos.y * billboard_size_.y;

    vertexPosition.y += 2.0f * billboard_size_.y;
    vertexPosition -= direction_wave * displacement_percentage;

    apex_percentage_frag = apex_percentage;
    gl_Position = projection * view * vec4(vertexPosition, 1.0f);

}