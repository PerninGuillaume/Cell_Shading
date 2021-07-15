#version 450

out vec4 FragColor;

in float distToCamera;
in vec4 FragPosLightSpace;

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
        FragColor = vec4(colFar * coeff, 1.0f);
    }
    else if (distToCamera > distFar1)
    {
        float fac = (distToCamera - distFar1) / (distFar2 - distFar1);
        fac = clamp(fac, 0, 1);
        FragColor = vec4(mix(colInter, colFar, fac) * coeff, 1.0f);
    }
    else
    {
        float fac = distToCamera / distFar1;
        fac = clamp(fac, 0, 1);
        FragColor = vec4(mix(colClose, colInter, fac) * coeff, 1.0f);
    }
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