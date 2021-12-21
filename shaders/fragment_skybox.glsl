#version 450
out vec4 FragColor;

in vec3 TexCoords;

uniform float blend_percentage = 0.f;
uniform vec3 additional_color = vec3(0.f, 0.f, 0.f);

uniform samplerCube skybox;

void main()
{
    vec4 texture_color = texture(skybox, TexCoords);
    FragColor = mix(texture_color, vec4(additional_color, 1.0f), blend_percentage);
}