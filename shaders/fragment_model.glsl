#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 color;

void main()
{
//    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    FragColor = texture(texture_diffuse1, TexCoords);
//    FragColor = vec4(color, 1.0f);
}