#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform float alpha_clip;
uniform vec3 color;
uniform bool use_color;

void main()
{
    if (use_color) {
        FragColor = vec4(color, 1.0f);
    }
    else
    {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        if (texColor.a < alpha_clip)
        discard;
        FragColor = texColor;
    }
    /*
    FragColor = vec4(vec3(texture(texture_diffuse1, TexCoords)) + color, 1.0f);
    FragColor = mix(texture(texture_diffuse1, TexCoords), vec4(color, 1.0f), 0.8f);
    */

}