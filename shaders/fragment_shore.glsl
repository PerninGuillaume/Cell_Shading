#version 450

out vec4 FragColor;

in vec2 TexCoords;
in float distToCamera;
in vec4 FragPosLightSpace;

uniform sampler2D tex_alpha_wave;
uniform sampler2D tex_black_wave;
uniform sampler2D tex_limit_wave;
uniform sampler2D tex_mask_wave;
uniform float alpha_clip;
uniform float offset;

//Shadow uniform
uniform bool use_shadow;
uniform bool pcf;
uniform float shadow_bias;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace);

void main()
{
    vec3 colFar = vec3(84.0f / 255.0f, 124.0f / 255.0f, 252.0f / 255.0f);
    vec3 colClose = vec3(10.0f / 255.0f, 98.0f / 255.0f, 224.0f / 255.0f);
    vec3 colInter = mix(colClose, colFar, 0.5);
    vec3 waterColor;
    int distFar2 = 400;
    int distFar1 = 300;
    float shadow_coeff = 0.0f;

    if (use_shadow) {
        shadow_coeff = ShadowCalculation(FragPosLightSpace);
    }
    float coeff = 1.0f - shadow_coeff;

    if (coeff != 1.0f) {
        coeff = min(1.0f, coeff + 0.3f);
    }

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

    texColor_wave.a = texColor_wave.a * texColor_mask.a;
    texColor_alpha_wave.a = 0.5;

    texColor_black_wave.a = 0.3;

    if (TexCoords.y < 0.05)
        discard;

    texColor_wave.a = texColor_wave.a * mix(0.0, 1.0, TexCoords.y) * mix(2.0, 0.0, TexCoords.y) * mix(1.0, 0.0, TexCoords.x);
    texColor_wave = texColor_wave + vec4(waterColor, 0.9 * texColor_mask.a);

    FragColor = vec4(vec3(texColor_wave * coeff), texColor_wave.a);
    if (TexCoords.y + abs(offset) < 0.4 && TexCoords.y < 0.45)
        FragColor.a = FragColor.a * (1- (abs(offset)));

}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
    return 0.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    // PCF
    float shadow;
    if (pcf) {
        shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - shadow_bias > pcfDepth  ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0f;
    } else {
        shadow = currentDepth - shadow_bias > closestDepth  ? 1.0 : 0.0;
    }

    return shadow;
}