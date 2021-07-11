#version 450
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // The z component is equal to the w component
    //So after the perspective divide the z component will be equal to 1 and be the
    //farthest object in the scene
}