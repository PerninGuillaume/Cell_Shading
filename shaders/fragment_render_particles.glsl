#version 450

in vec3 Position;

uniform vec4 Color;

layout( location = 0 ) out vec4 FragColor;


void main() {
  FragColor = mix(Color, vec4(1, 0.5, 0, 1), 1 - Position.y/ 5);
  FragColor.a = mix(0.5, 0, Position.y / 10);
  // FragColor = Color;
}