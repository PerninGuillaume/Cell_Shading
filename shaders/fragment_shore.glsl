#version 450

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

const int NB_CASCADES = NB_CASCADES_TO_REPLACE;

out vec4 FragColor;

in vec2 TexCoords;
in float distToCamera;
in vec3 FragPosWorldSpace;
in vec4 FragPosLightSpace_cascade[NB_CASCADES];

uniform sampler2D tex_alpha_wave;
uniform sampler2D tex_black_wave;
uniform sampler2D tex_limit_wave;
uniform sampler2D tex_mask_wave;
uniform float offset;
uniform DirLight dirLight;

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

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 col, float shadow);

void main()
{
    vec3 colFar = vec3(84.0f / 255.0f, 124.0f / 255.0f, 252.0f / 255.0f);
    vec3 colClose = vec3(10.0f / 255.0f, 98.0f / 255.0f, 224.0f / 255.0f);
    vec3 colInter = mix(colClose, colFar, 0.5);
    vec3 waterColor;
    int distFar2 = 400;
    int distFar1 = 300;
    float shadow_coeff = 0.0f;

    vec3 lightDir = normalize(-dirLight.direction);
    vec3 normal = vec3(0.f, 1.f, 0.f);
    if (use_shadow) {
        vec3 debug_color_shadow;
        shadow_coeff = ShadowCalculation(view, FragPosWorldSpace, NB_CASCADES, cascade_z_limits, blend_between_cascade
        , normal, FragPosLightSpace_cascade, lightDir, shadow_biases, pcf, square_sample_size, color_cascade_layer
        , shadowMap_cascade, debug_color_shadow);
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

    FragColor = vec4(vec3(texColor_wave), texColor_wave.a);
    if (TexCoords.y + abs(offset) < 0.4 && TexCoords.y < 0.45)
        FragColor.a = FragColor.a * (1- (abs(offset)));

    vec3 result = CalcDirLight(dirLight, normal, FragColor.rgb, shadow_coeff);

    FragColor = vec4(result, FragColor.a);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 col, float shadow) {
    //ambient
    vec3 ambient = light.ambient * col;

    // diffuse
    vec3 lightDirection = normalize(-light.direction);
    float coeff = max(dot(normal, lightDirection), 0.0f);

    vec3 diffuse = light.diffuse * coeff * col;

    //total
    vec3 result = ambient + (1.0 - shadow) * diffuse;

    return result;
}
