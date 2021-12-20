#version 450

uniform sampler2D texture_diffuse1;
uniform float alpha_clip;

in vec2 TexCoords;

void main() {
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if (texColor.a < alpha_clip) {
        discard;
    }
    gl_FragDepth = gl_FragCoord.z;
}
