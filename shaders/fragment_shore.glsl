#version 450

out vec4 FragColor;

in vec2 TexCoords;
in float distToCamera;

uniform sampler2D tex_alpha_wave;
uniform sampler2D tex_black_wave;
uniform sampler2D tex_limit_wave;
uniform sampler2D tex_mask_wave;
uniform float alpha_clip;
uniform float offset;

void main()
{
    vec3 colFar = vec3(84.0f / 255.0f, 124.0f / 255.0f, 252.0f / 255.0f);
    vec3 colClose = vec3(10.0f / 255.0f, 98.0f / 255.0f, 224.0f / 255.0f);
    vec3 colInter = mix(colClose, colFar, 0.5);
    vec3 waterColor;
    int distFar2 = 400;
    int distFar1 = 300;

    if (distToCamera > distFar2)
    {
        waterColor = colFar;
    }
    else if (distToCamera > distFar1)
    {
        float fac = (distToCamera - distFar1) / (distFar2 - distFar1);
        fac = clamp(fac, 0, 1);
        waterColor = mix(colInter, colFar, fac);
    }
    else
    {
        float fac = distToCamera / distFar1;
        fac = clamp(fac, 0, 1);
        waterColor = mix(colClose, colInter, fac);
    }

    vec4 texColor_wave = texture(tex_limit_wave, vec2(TexCoords.x + offset/ 2, TexCoords.y + offset / 2));
    texColor_wave += texture(tex_limit_wave, vec2(TexCoords.x - offset / 2, TexCoords.y + offset / 2));
    vec4 texColor_mask = texture(tex_mask_wave, TexCoords);
    vec4 texColor_alpha_wave = texture(tex_alpha_wave, TexCoords);
    vec4 texColor_black_wave = texture(tex_black_wave, TexCoords);
//    vec4 texColor_mask = texture(tex_cloud_mask, vec2((TexCoords.x + offset), TexCoords.y));

//    texColor_cloud.a = texColor_cloud.a * texColor_mask.a * mix(1.2, 0.0, TexCoords.x) * mix(0.0, 1.2, TexCoords.x);

//    if (texColor_cloud.a < alpha_clip)
//    discard;
    texColor_wave.a = texColor_wave.a * texColor_mask.a;
    texColor_alpha_wave.a = 0.5;
//    texColor_wave.rgb = texColor_wave.rgb + texColor_alpha_wave.rgb + texColor_black_wave.rgb;

//    if (texColor_wave.a <= 0.1 && TexCoords.y + offset < 0.1)
//        discard;
    texColor_black_wave.a = 0.3;

    if (TexCoords.y < 0.05)
        discard;

    texColor_wave.a = texColor_wave.a * mix(0.0, 1.0, TexCoords.y) * mix(2.0, 0.0, TexCoords.y) * mix(1.0, 0.0, TexCoords.x);
    texColor_wave = texColor_wave + vec4(waterColor, 0.9 * texColor_mask.a);
    FragColor = texColor_wave;
    if (TexCoords.y + abs(offset) < 0.4 && TexCoords.y < 0.45)
        FragColor.a = FragColor.a * (1- (abs(offset)));

//    if (TexCoords.x <= 0.1)
//        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
//    if (TexCoords.x >= 0.9)
//        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
//    if (TexCoords.x >= 0.49 && TexCoords.x <= 0.51)
//        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}