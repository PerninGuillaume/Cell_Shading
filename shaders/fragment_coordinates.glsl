#version 450

in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mix_ratio;

out vec4 FragColor;

void main() {
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), mix_ratio);
}
