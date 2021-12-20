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
uniform float shadow_biases[NB_CASCADES];
uniform bool use_color;
uniform bool no_texture;
uniform bool pcf;
uniform int square_sample_size;
uniform bool color_cascade_layer;
uniform bool blend_between_cascade;

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap_cascade[NB_CASCADES];

out vec4 FragColor;

vec3 debug_color = vec3(0.f);
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

float percentage_sampling[2];
int layer_index_sampling[2];

void compute_percentage_sampling_shadow_map(int layer) {
    vec4 FragPosViewSpace_homegeneous = view * vec4(FragPosWorldSpace, 1.0f);
    vec3 FragPosViewSpace = FragPosViewSpace_homegeneous.xyz / FragPosViewSpace_homegeneous.w; // Don't think that is useful as the view does not change the 4th component
    float depth = abs(FragPosViewSpace.z); // abs useful ?

    float band_blend_percentage = 0.05f;
    float extent_z_current_cascade = cascade_z_limits[layer + 1] - cascade_z_limits[layer];
    float size_current_band_blend = extent_z_current_cascade * band_blend_percentage;
    float percentage_in_current_cascade = (depth - cascade_z_limits[layer]) / extent_z_current_cascade;
    float inverse_band_blend_percentage = 1.f - band_blend_percentage;

    if (percentage_in_current_cascade < band_blend_percentage && layer != 0) { // Just after beginning of cascade
        //if (color_cascade_layer)
        //    debug_color -= vec3(0.5f);

        float size_previous_band_blend = (cascade_z_limits[layer] - cascade_z_limits[layer - 1]) * band_blend_percentage;
        float percentage_in_current_band = (depth - cascade_z_limits[layer] + size_previous_band_blend) / (size_current_band_blend + size_previous_band_blend);
        // map from [0,0.05] to [0.5,1.0]
        //percentage_in_current_band = percentage_in_current_cascade * 0.5f / band_blend_percentage + 0.5f;

        if (color_cascade_layer)
            debug_color[0] = percentage_in_current_band;

        percentage_sampling[0] = percentage_in_current_band;
        percentage_sampling[1] = 1.f - percentage_in_current_band;
        layer_index_sampling[0] = layer;
        layer_index_sampling[1] = layer - 1;
    }
    else if (percentage_in_current_cascade > inverse_band_blend_percentage && layer != NB_CASCADES - 1) { // Just before beginning of cascade

        float size_next_band_blend = (cascade_z_limits[layer + 2] - cascade_z_limits[layer + 1]) * band_blend_percentage;
        float percentage_in_current_band = (depth - (cascade_z_limits[layer + 1] - size_current_band_blend)) / (size_current_band_blend + size_next_band_blend);
        // map from [0.95,1.0] to [0,0.5]
        //percentage_in_current_band = (percentage_in_current_cascade - inverse_band_blend_percentage) * 0.5f / band_blend_percentage;

        if (color_cascade_layer)
            debug_color[1] = percentage_in_current_band;

        percentage_sampling[0] = 1.f - percentage_in_current_band;
        percentage_sampling[1] = percentage_in_current_band;
        layer_index_sampling[0] = layer;
        layer_index_sampling[1] = layer + 1;
    }
    else {
        if (color_cascade_layer)
            debug_color[2] = percentage_in_current_cascade;
        layer_index_sampling[0] = layer;
        layer_index_sampling[1] = -1;
        percentage_sampling[0] = 1.f;
        percentage_sampling[1] = 0.f;
    }
}

float shadow_percentage(int layer, vec3 normal)
{
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
    vec3 lightDir = normalize(-dirLight.direction);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    bias *= shadow_biases[layer];
    // PCF
    float shadow;
    if (pcf) {
        shadow = 0.0;
        float nb_samples = pow(square_sample_size * 2 + 1, 2);
        vec2 texelSize = 1.0 / textureSize(shadowMap_cascade[layer], 0);
        for (int x = -square_sample_size; x <= square_sample_size; ++x)
        {
            for (int y = -square_sample_size; y <= square_sample_size; ++y)
            {
                float pcfDepth = texture(shadowMap_cascade[layer], projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
            }
        }
        shadow /= nb_samples;
    } else {
        shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    }

    return shadow;
}

float ShadowCalculation(vec3 normal)
{
    int layer = find_cascaded_layer();
    if (layer == NB_CASCADES)
        return 0.0; // Depth behind all Shadow layer

    if (!blend_between_cascade)
        return shadow_percentage(layer, normal);

    compute_percentage_sampling_shadow_map(layer);

    float total_shadow = 0.f;
    total_shadow += percentage_sampling[0] * shadow_percentage(layer_index_sampling[0], normal);
    if (layer_index_sampling[1] != -1)
        total_shadow += percentage_sampling[1] * shadow_percentage(layer_index_sampling[1], normal);


    return total_shadow;
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
        int layer = find_cascaded_layer();

        shadow = ShadowCalculation(normal);

        if (color_cascade_layer) {
            if (blend_between_cascade) {
                FragColor = vec4(debug_color.r, debug_color.g, debug_color.b, 1.f);
                return;
            }

            if (layer == NB_CASCADES)
                debug_color = vec3(-0.5f);
            else if (layer == 0)
                debug_color.r = 0.1f;
            else if (layer == 1)
                debug_color.g = 0.1f;
            else if (layer == 2)
                debug_color.b = 0.1f;

        }
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
