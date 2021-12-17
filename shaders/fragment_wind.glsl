#version 450

in float intensity;

out vec4 FragColor;

void main() {
    //FragColor = vec4(vec3(1.0f), 1.0f);
    FragColor = vec4(vec3(1.0f), intensity);
}
