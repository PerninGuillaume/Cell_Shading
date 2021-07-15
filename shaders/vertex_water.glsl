#version 450

layout (location = 0) in vec3 aPos;


out vec2 TexCoords;
out float distToCamera;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    vec3 FragPos = vec3(model * vec4(aPos, 1.0f));
    vec4 cs_position = view * model * vec4(aPos, 1.0);
    distToCamera = -cs_position.z;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0f);
    gl_Position = projection * cs_position;
}