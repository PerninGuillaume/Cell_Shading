#version 450

const int NB_CASCADES = NB_CASCADES_TO_REPLACE;

out vec4 FragColor;

in float distToCamera;
in vec3 FragPosWorldSpace;
in vec4 FragPosLightSpace_cascade[NB_CASCADES];

uniform vec3 dirLight;

// Shadow Variables
uniform mat4 view;
uniform bool use_shadow;
uniform float cascade_z_limits[NB_CASCADES + 1]; // example value for nb_cascades = 2 : [0.1f, 100.f, 1000.f]
uniform float shadow_biases[NB_CASCADES];
uniform bool use_color;
uniform bool no_texture;
uniform bool pcf;
uniform int square_sample_size;
uniform bool color_cascade_layer;
uniform bool blend_between_cascade;

uniform sampler2D shadowMap_cascade[NB_CASCADES];

float ShadowCalculation(mat4 view, vec3 FragPosWorldSpace, int nb_cascades, float cascade_z_limits[NB_CASCADES + 1]
, bool blend_between_cascade, vec3 normal, vec4 FragPosLightSpace_cascade[NB_CASCADES], vec3 lightDir, float shadow_biases[NB_CASCADES]
, bool pcf, int square_sample_size, bool color_cascade_layer, sampler2D shadowMap_cascade[NB_CASCADES], out vec3 debug_color_out);

void main()
{
    vec3 colFar = vec3(84.0f / 255.0f, 124.0f / 255.0f, 252.0f / 255.0f);
    vec3 colClose = vec3(10.0f / 255.0f, 98.0f / 255.0f, 224.0f / 255.0f);
    vec3 colInter = mix(colClose, colFar, 0.5);

    int distFar2 = 400;
    int distFar1 = 300;

    float shadow_coeff = 0.0f;
    vec3 debug_color_shadow;
    if (use_shadow) {
        vec3 normal = vec3(0.f, 1.f, 0.f);
        vec3 lightDir = normalize(-dirLight);
        shadow_coeff = ShadowCalculation(view, FragPosWorldSpace, NB_CASCADES, cascade_z_limits, blend_between_cascade
        , normal, FragPosLightSpace_cascade, lightDir, shadow_biases, pcf, square_sample_size, color_cascade_layer
        , shadowMap_cascade, debug_color_shadow);
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
    FragColor.rgb += debug_color_shadow;
}