#version 450

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

const int NB_CASCADES = NB_CASCADES_TO_REPLACE;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPosWorldSpace;
in vec4 FragPosLightSpace_cascade[NB_CASCADES];
in vec3 light_gouraud;

uniform mat4 view;

// Light uniforms
uniform DirLight dirLight;
uniform vec3 color;
uniform float zAtoon[256];
uniform float alpha_clip;
uniform bool use_zAtoon;
uniform bool use_color;
uniform bool no_texture;
uniform sampler2D texture_diffuse1;

// shadow uniforms
uniform bool use_shadow;
uniform float cascade_z_limits[NB_CASCADES + 1]; // example value for nb_cascades = 2 : [0.1f, 100.f, 1000.f]
uniform float shadow_biases[NB_CASCADES];
uniform bool pcf;
uniform int square_sample_size;
uniform bool color_cascade_layer;
uniform bool blend_between_cascade;

uniform sampler2D first_shadowMap_cascade;
uniform sampler2D second_shadowMap_cascade;
uniform sampler2D third_shadowMap_cascade;

out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 col, float shadow);

float ShadowCalculation(mat4 view, vec3 FragPosWorldSpace, int nb_cascades, float cascade_z_limits[NB_CASCADES + 1]
, bool blend_between_cascade, vec3 normal, vec4 FragPosLightSpace_cascade[NB_CASCADES], vec3 lightDir, float shadow_biases[NB_CASCADES]
, bool pcf, int square_sample_size, bool color_cascade_layer, sampler2D first_shadowMap_cascade
, sampler2D second_shadowMap_cascade, sampler2D third_shadowMap_cascade, out vec3 debug_color_out);

void main() {
    vec3 col = color;
    if (use_color) {
        col = color;
    } else {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        if (texColor.a < alpha_clip) {
            discard;
        }
        col = vec3(texColor);
    }
    if (no_texture) {
        col = vec3(0.5f);
    }
    vec3 debug_color_shadow = vec3(0.f);
    vec3 normal = normalize(Normal);
    float shadow = 0.0f;
    if (use_shadow) {
        vec3 lightDir = normalize(-dirLight.direction);
        shadow = ShadowCalculation(view, FragPosWorldSpace, NB_CASCADES, cascade_z_limits, blend_between_cascade
        , normal, FragPosLightSpace_cascade, lightDir, shadow_biases, pcf, square_sample_size, color_cascade_layer
        , first_shadowMap_cascade, second_shadowMap_cascade, third_shadowMap_cascade, debug_color_shadow);
    }
    vec3 result = CalcDirLight(dirLight, normal, col, shadow);

    FragColor = vec4(result + debug_color_shadow, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 col, float shadow) {
    //ambient
    vec3 ambient = light.ambient * col;

    // diffuse
    vec3 lightDirection = normalize(-light.direction);
    float coeff = max(dot(normal, lightDirection), 0.0f);
    if (use_zAtoon) {
        coeff = zAtoon[int(coeff * 256.0f)];
    }

    vec3 diffuse = light.diffuse * coeff * col;

    //total
    vec3 result = ambient + (1.0 - shadow) * diffuse;

    return result;
}
