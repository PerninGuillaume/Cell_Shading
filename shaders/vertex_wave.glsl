#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out float apex_percentage_frag;

const int nb_waves = 200;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_up = vec3(0.f, 1.f, 0.f);
uniform vec3 camera_right = vec3(1.f, 0.f, 0.f);
uniform vec3 wave_center[nb_waves];
uniform vec3 billboard_size[nb_waves];
uniform vec3 center_waves;
uniform float displacement_percentage[nb_waves];
uniform float apex_percentage[nb_waves];

void main()
{
    TexCoords = aTexCoords;

    vec3 direction_wave = normalize(wave_center[gl_InstanceID] - center_waves) * 10.0f;

    vec3 billboard_size_ = billboard_size[gl_InstanceID];
    vec3 vertexPosition = wave_center[gl_InstanceID]
    + camera_right * aPos.x * billboard_size_.x
    + camera_up * aPos.y * billboard_size_.y;

    vertexPosition.y += 2.0f * billboard_size_.y;
    vertexPosition -= direction_wave * displacement_percentage[gl_InstanceID];

    apex_percentage_frag = apex_percentage[gl_InstanceID];
    //vertexPosition = vec3(20.f, 20.f, 20.f);
    //vertexPosition = aPos;
    gl_Position = projection * view * vec4(vertexPosition, 1.0f);

}