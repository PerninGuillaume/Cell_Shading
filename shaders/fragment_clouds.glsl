#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D tex_cloud;
uniform sampler2D tex_cloud_mask;
uniform float alpha_clip;
uniform float offset;

void main()
{
    vec4 texColor_cloud = texture(tex_cloud, vec2((TexCoords.x + offset), TexCoords.y));
    vec4 texColor_mask = texture(tex_cloud_mask, vec2((TexCoords.x + offset), TexCoords.y));

    texColor_cloud.a = texColor_cloud.a * texColor_mask.a * mix(1.2, 0.0, TexCoords.x) * mix(0.0, 1.2, TexCoords.x);
//    if (texColor_cloud.a < alpha_clip)
//    discard;
    FragColor = texColor_cloud;
//    if (TexCoords.x <= 0.1 || TexCoords.x >= 0.9)
//        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
//    if (TexCoords.x >= 0.49 && TexCoords.x <= 0.51)
//        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}