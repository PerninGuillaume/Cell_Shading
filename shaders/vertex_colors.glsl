#version 450
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPosition;

out vec3 Normal;
out vec3 FragPos; //Frag pos is in the view coordinates
out vec3 LightPos;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    Normal = mat3(transpose(inverse(view * model))) * aNormal; //To have proper normal if we scale the object
    FragPos = vec3(view * model * vec4(aPos, 1.0f));
    LightPos = vec3(view * vec4(lightPosition, 1.0));
}
