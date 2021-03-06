#version 450

const int NB_CASCADES = NB_CASCADES_TO_REPLACE;

vec3 debug_color = vec3(0.f);
// This function finds in which part of the divided camera frustrum the current fragment is in.
int find_cascaded_layer(mat4 view, vec3 FragPosWorldSpace, int nb_cascades, float cascade_z_limits[NB_CASCADES + 1]) {
    vec4 FragPosViewSpace_homegeneous = view * vec4(FragPosWorldSpace, 1.0f);
    vec3 FragPosViewSpace = FragPosViewSpace_homegeneous.xyz / FragPosViewSpace_homegeneous.w; // Don't think that is useful as the view does not change the 4th component
    float depth = abs(FragPosViewSpace.z); // abs useful ?
    for (int i = 0; i < nb_cascades; ++i) {
        if (depth < cascade_z_limits[i + 1])
        return i;
    }
    return nb_cascades;
}

float percentage_sampling[2];
int layer_index_sampling[2];

void compute_percentage_sampling_shadow_map(int layer, mat4 view, vec3 FragPosWorldSpace, float cascade_z_limits[NB_CASCADES + 1]
, bool color_cascade_layer) {
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

        if (color_cascade_layer)
            debug_color -= vec3(0.5f);

        percentage_sampling[0] = percentage_in_current_band;
        percentage_sampling[1] = 1.f - percentage_in_current_band;
        layer_index_sampling[0] = layer;
        layer_index_sampling[1] = layer - 1;
    }
    else if (percentage_in_current_cascade > inverse_band_blend_percentage && layer != NB_CASCADES - 1) { // Just before beginning of cascade

        float size_next_band_blend = (cascade_z_limits[layer + 2] - cascade_z_limits[layer + 1]) * band_blend_percentage;
        float percentage_in_current_band = (depth - (cascade_z_limits[layer + 1] - size_current_band_blend)) / (size_current_band_blend + size_next_band_blend);

        if (color_cascade_layer)
            debug_color -= vec3(0.5f);

        percentage_sampling[0] = 1.f - percentage_in_current_band;
        percentage_sampling[1] = percentage_in_current_band;
        layer_index_sampling[0] = layer;
        layer_index_sampling[1] = layer + 1;
    }
    else {
        layer_index_sampling[0] = layer;
        layer_index_sampling[1] = -1;
        percentage_sampling[0] = 1.f;
        percentage_sampling[1] = 0.f;
    }
}

float shadow_percentage(int layer, vec3 normal, vec4 FragPosLightSpace_cascade[NB_CASCADES]
, sampler2D shadowMap_cascade[NB_CASCADES], vec3 lightDir, float shadow_biases[NB_CASCADES], bool pcf, int square_sample_size)
{
    vec4 fragPosLightSpace = FragPosLightSpace_cascade[layer];

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) {
        return 0.0;
    }
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap_cascade[layer], projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
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

float ShadowCalculation(mat4 view, vec3 FragPosWorldSpace, int nb_cascades, float cascade_z_limits[NB_CASCADES + 1]
, bool blend_between_cascade, vec3 normal, vec4 FragPosLightSpace_cascade[NB_CASCADES], vec3 lightDir, float shadow_biases[NB_CASCADES]
, bool pcf, int square_sample_size, bool color_cascade_layer, sampler2D shadowMap_cascade[NB_CASCADES], out vec3 debug_color_out)
{
    int layer = find_cascaded_layer(view, FragPosWorldSpace, nb_cascades, cascade_z_limits);

    if (color_cascade_layer) {
        if (layer == NB_CASCADES)
            debug_color = vec3(-0.5f);
        else if (layer == 0)
            debug_color.r += 0.2f;
        else if (layer == 1)
            debug_color.g += 0.2f;
        else if (layer == 2)
            debug_color.b += 0.2f;
    }

    if (layer == NB_CASCADES) {
        return 0.0;// Depth behind all Shadow layer
    }

    if (!blend_between_cascade) {
        return shadow_percentage(layer, normal, FragPosLightSpace_cascade, shadowMap_cascade, lightDir, shadow_biases, pcf, square_sample_size);
    }

    compute_percentage_sampling_shadow_map(layer, view, FragPosWorldSpace, cascade_z_limits, color_cascade_layer);

    float total_shadow = 0.f;
    total_shadow += percentage_sampling[0]
    * shadow_percentage(layer_index_sampling[0], normal, FragPosLightSpace_cascade, shadowMap_cascade, lightDir, shadow_biases, pcf, square_sample_size);
    if (layer_index_sampling[1] != -1) {
        total_shadow += percentage_sampling[1]
        * shadow_percentage(layer_index_sampling[1], normal, FragPosLightSpace_cascade, shadowMap_cascade, lightDir, shadow_biases, pcf, square_sample_size);
    }
    debug_color_out = debug_color;

    return total_shadow;
}