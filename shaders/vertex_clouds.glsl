#version 450

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;

    mat4 mv = view * model;
//    float d = sqrt(mv[0][0] * mv[0][0] + mv[1][1] * mv[1][1] + mv[2][2] * mv[2][2]);
    mv[0][0] = 1;
    mv[0][1] = 0;
    mv[0][2] = 0;
    mv[1][0] = 0;
    mv[1][1] = 1;
    mv[1][2] = 0;
    mv[2][0] = 0;
    mv[2][1] = 0;
    mv[2][2] = 1;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}