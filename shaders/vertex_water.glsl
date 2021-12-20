#version 450

layout (location = 0) in vec3 aPos;

const int NB_CASCADES = NB_CASCADES_TO_REPLACE;

out vec2 TexCoords;
out float distToCamera;
out vec3 FragPosWorldSpace;
out vec4 FragPosLightSpace_cascade[NB_CASCADES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrices_cascade[NB_CASCADES];

void main()
{
    FragPosWorldSpace = vec3(model * vec4(aPos, 1.0f));
    vec4 cs_position = view * model * vec4(aPos, 1.0);
    distToCamera = -cs_position.z;
    for (int i = 0; i < NB_CASCADES; ++i) {
        FragPosLightSpace_cascade[i] = lightSpaceMatrices_cascade[i] * vec4(FragPosWorldSpace, 1.0);
    }
    gl_Position = projection * cs_position;
}