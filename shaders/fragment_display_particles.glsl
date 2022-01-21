#version 450

in vec2 TexCoord;

layout( binding = 0 ) uniform sampler2D Tex;

layout( location = 0 ) out vec4 FragColor;

void main() {
  vec4 texColor = texture( Tex, TexCoord );
  FragColor = texColor;

//  FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
