#version 450

out vec4 FragColor;

void main()
{
//        FragColor = vec4(vec3(200.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f), 1.0f);
//
        FragColor = vec4(vec3(10.0f / 255.0f, 98.0f / 255.0f, 224.0f / 255.0f), 1.0f);
    /*
    FragColor = vec4(vec3(texture(texture_diffuse1, TexCoords)) + color, 1.0f);
    FragColor = mix(texture(texture_diffuse1, TexCoords), vec4(color, 1.0f), 0.8f);
    */

}