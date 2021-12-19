#version 450

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

const int NB_CASCADES = 2;

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPosWorldSpace;
in vec4 FragPosLightSpace_cascade[NB_CASCADES];
//in vec4 FragPosLightSpace;

uniform mat4 view;
uniform DirLight dirLight;
uniform vec3 color;
uniform float zAtoon[256];
uniform float alpha_clip;
uniform float shadow_bias;
uniform bool use_zAtoon;
uniform bool use_shadow;
uniform float cascade_z_limits[NB_CASCADES + 1]; // example value for nb_cascades = 2 : [0.1f, 100.f, 1000.f]
uniform bool use_color;
uniform bool no_texture;
uniform bool pcf;
uniform bool color_cascade_layer;

uniform sampler2D texture_diffuse1;
//uniform sampler2D shadowMap;
uniform sampler2D shadowMap_cascade[NB_CASCADES];

out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 col, float shadow);

// This function finds in which part of the divided camera frustrum the current fragment is in.
int find_cascaded_layer() {
    vec4 FragPosViewSpace_homegeneous = view * vec4(FragPosWorldSpace, 1.0f);
    vec3 FragPosViewSpace = FragPosViewSpace_homegeneous.xyz / FragPosViewSpace_homegeneous.w; // Don't think that is useful as the view does not change the 4th component
    float depth = abs(FragPosViewSpace.z); // abs useful ?
    for (int i = 0; i < NB_CASCADES; ++i) {
        if (depth < cascade_z_limits[i + 1])
            return i;
    }
    return NB_CASCADES;
}

float ShadowCalculation()
{
    int layer = find_cascaded_layer();
    if (layer == NB_CASCADES)
        return 0.0; // Depth behind all Shadow layer
    vec4 fragPosLightSpace = FragPosLightSpace_cascade[layer];

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
        return 0.0;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap_cascade[layer], projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    // PCF
    float shadow;
    if (pcf) {
        shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap_cascade[layer], 0);
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap_cascade[layer], projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - shadow_bias > pcfDepth  ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0f;
    } else {
        shadow = currentDepth - shadow_bias > closestDepth  ? 1.0 : 0.0;
    }

    return shadow;
}

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
    vec3 normal = normalize(Normal);
    float shadow = 0.0f;
    vec3 debug_color = vec3(0.f);
    if (use_shadow) {
        int layer = find_cascaded_layer();

        if (color_cascade_layer) {
            if (layer == 0)
                debug_color.r = 1.0f;
            else if (layer == 1)
                debug_color.g = 1.0f;
            else if (layer == 2)
                debug_color.b = 1.0f;
        }

        shadow = ShadowCalculation();
    }
    vec3 result = CalcDirLight(dirLight, normal, col, shadow);

    FragColor = vec4(result + debug_color, 1.0f);
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
