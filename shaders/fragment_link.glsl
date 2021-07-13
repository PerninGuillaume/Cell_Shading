#version 450

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform DirLight dirLight;
uniform vec3 viewPosition;
uniform vec3 color;
uniform float zAtoon[256];
uniform float alpha_clip;
uniform float shadow_bias;
uniform bool use_zAtoon;
uniform bool use_shadow;
uniform bool use_color;
uniform bool no_texture;
uniform bool pcf;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;

out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 col, float shadow);

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
    if (use_shadow) {
        shadow = ShadowCalculation(FragPosLightSpace);
    }
    vec3 result = CalcDirLight(dirLight, normal, col, shadow);

    FragColor = vec4(result, 1.0f);
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
