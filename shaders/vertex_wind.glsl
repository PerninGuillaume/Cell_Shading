#version 450

layout (location = 0) in vec3 aPos;

const int nb_blow_of_wind = 20;
uniform vec3 offsets[nb_blow_of_wind];
uniform vec3 directions[nb_blow_of_wind];
uniform int in_animation[nb_blow_of_wind];
uniform float animation_percentage[nb_blow_of_wind];
uniform float phase_vector[nb_blow_of_wind];

out vec3 dir;
out int animated;
out float anim_percentage;
out float phase;
out int spiral;

void main()
{
    dir = normalize(directions[gl_InstanceID]);
    animated = in_animation[gl_InstanceID];
    anim_percentage = animation_percentage[gl_InstanceID];
    phase = phase_vector[gl_InstanceID];
    spiral = int(gl_InstanceID % 3 == 0);

    gl_Position = vec4(aPos + offsets[gl_InstanceID], 1.0);
}