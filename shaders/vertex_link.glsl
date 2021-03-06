#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

const int NB_CASCADES = NB_CASCADES_TO_REPLACE;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrices_cascade[NB_CASCADES];

out vec3 FragPosWorldSpace;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace_cascade[NB_CASCADES];

void main() {
    FragPosWorldSpace = vec3(model * vec4(aPos, 1.0f));
    Normal = mat3(transpose(inverse(model))) * aNormal; //To have proper normal if we scale the object
    TexCoords = aTexCoords;

    for (int i = 0; i < NB_CASCADES; ++i) {
        FragPosLightSpace_cascade[i] = lightSpaceMatrices_cascade[i] * vec4(FragPosWorldSpace, 1.0);
    }
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
