#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sun_border;
uniform sampler2D sun_eclat;
uniform sampler2D sun_center;
uniform sampler2D sun_inner_ring;
uniform sampler2D sun_gradient;
uniform float alpha_clip;
uniform vec3 color_border;
uniform vec3 color_center;
uniform vec3 color_inner_ring;
uniform vec3 color_gradient;

void main()
{
    float alpha_clip = 0.1f;
    vec4 texcolor_border = texture(sun_border, TexCoords);
    //vec4 texColor_eclat = texture(sun_eclat, TexCoords);
    vec4 texColor_center = texture(sun_center, TexCoords);
    vec4 texColor_inner_ring = texture(sun_inner_ring, TexCoords);
    vec4 texColor_gradient = texture(sun_gradient, TexCoords);


    vec3 color_border_frag = texcolor_border.a * color_border;
    //vec4 color_eclat = texColor_eclat;
    vec3 color_center_frag = texColor_center.a * color_center;
    //vec3 color_inner_ring_frag = texColor_inner_ring.a * color_inner_ring;
    vec3 color_gradient_frag = texColor_gradient.a * color_gradient;

    //if (texcolor_border.a < 0.1)
    //    discard;
    //FragColor = vec4(mix(vec3(texcolor_border), color, 0.2f), 1.0f);
    //FragColor = vec4(texcolor_border.a * color, 1.0f);
    //FragColor = texcolor_border + texColor_eclat + texColor_center + texColor_inner_ring + texColor_gradient;
    //Trying to use the inner_ring to set the blending
    FragColor = vec4(color_center_frag + color_gradient_frag
        , texColor_inner_ring.a);
    //FragColor = vec4(texColor_center.a, texColor_inner_ring.a, texColor_gradient.a, 1.0f);
    //vec3 color_base = texColor_center.a * vec3(0.9f);
    //FragColor = vec4(vec3(texColor_center.a, texColor_inner_ring.a, texColor_gradient.a) * color_center, 1.0f);
    /*
    FragColor = vec4(vec3(texture(texture_diffuse1, TexCoords)) + color, 1.0f);
    FragColor = mix(texture(texture_diffuse1, TexCoords), vec4(color, 1.0f), 0.8f);
    */

}