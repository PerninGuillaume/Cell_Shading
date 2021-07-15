#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D tex_wave;
uniform float alpha_clip;
uniform float apex_percentage;

void main()
{
    if (apex_percentage < 0.0f) {
        discard;
    }
    vec4 texColor_cloud = texture(tex_wave, TexCoords);
    FragColor = vec4(texColor_cloud.a);
    if (texColor_cloud.r != 1.0f || texColor_cloud.g != 1.0f || texColor_cloud.b != 1.0f || texColor_cloud.a != 1.0f) {
        if (texColor_cloud.a == 0.0f || texColor_cloud.r <= 34.0f) {
            discard;
        }
    }
    FragColor = vec4(vec3(texColor_cloud), apex_percentage * texColor_cloud.a);
}