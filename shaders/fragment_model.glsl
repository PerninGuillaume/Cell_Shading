#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec3 color;
uniform bool use_color;

void main()
{
    if (use_color)
        FragColor = vec4(color, 1.0f);
    else
        FragColor = texture(texture_diffuse1, TexCoords);
    /*
    FragColor = vec4(vec3(texture(texture_diffuse1, TexCoords)) + color, 1.0f);
    FragColor = mix(texture(texture_diffuse1, TexCoords), vec4(color, 1.0f), 0.8f);
    */

}