#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float displacement;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main() {
    vec3 displaced_aPos = aPos + aNormal * displacement;
    gl_Position = projection * view * model * vec4(displaced_aPos, 1.0f);
    Normal = mat3(transpose(inverse(model))) * aNormal; //To have proper normal if we scale the object
    FragPos = vec3(model * vec4(aPos, 1.0f));
    TexCoords = aTexCoords;
}