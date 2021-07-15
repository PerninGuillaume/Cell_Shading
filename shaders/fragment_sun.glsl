#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sun_border;
uniform sampler2D sun_eclat;
uniform sampler2D sun_center;
uniform sampler2D sun_inner_ring;
uniform sampler2D sun_gradient;
uniform vec3 color_border;
uniform vec3 color_center;
uniform vec3 color_inner_ring;
uniform vec3 color_gradient;

void main()
{
    vec4 texcolor_border = texture(sun_border, TexCoords);
    vec4 texColor_center = texture(sun_center, TexCoords);
    vec4 texColor_inner_ring = texture(sun_inner_ring, TexCoords);
    vec4 texColor_gradient = texture(sun_gradient, TexCoords);


    vec3 color_border_frag = texcolor_border.a * color_border;
    vec3 color_center_frag = texColor_center.a * color_center;
    vec3 color_gradient_frag = texColor_gradient.a * color_gradient;

    FragColor = vec4(color_center_frag + color_gradient_frag
        , texColor_inner_ring.a);

}