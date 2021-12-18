#version 450
layout (points) in;
layout (triangle_strip, max_vertices=1024) out;

in vec3 dir[];
in int animated[];
in float anim_percentage[];
in float phase[];

out float intensity;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    if (animated[0] == 0)
        return;

    //0 for just starting, 1 for finished
    float animation_percentage = anim_percentage[0];

    vec4 pos_line_straight = gl_in[0].gl_Position;

    // polynomial equal to 0 at 0, 1 at 0.5, 0 at 1
    float overall_alpha = -4.f * animation_percentage * animation_percentage + 4.f * animation_percentage;

    int max_rectangles = 102;
    float size_wind = 50.f;
    float increment = size_wind / max_rectangles;
    vec3 dir_wind = normalize(dir[0]);
    vec4 offset_dir = vec4(normalize(cross(dir[0], vec3(0.0f, 0.0f, 1.0f))), 0.0f);

    float delta_apparition = 0.3f;
    for (int i = 0; i < max_rectangles; ++i) {
        float advance_percentage = float(i) / float(max_rectangles);
        float advance_percentage_bis = advance_percentage * (1.0f + 2 * delta_apparition) - delta_apparition;
        // Advance_percentage_bis goes from -delta_apparition to 1 + delta_apparition
        // It serves to regulate how the opcaity of a point on the line appears
        if (advance_percentage_bis < animation_percentage)
            intensity = smoothstep(animation_percentage - delta_apparition, animation_percentage - 0.2f, advance_percentage_bis);
        else {
            intensity = smoothstep(animation_percentage + 0.2f, animation_percentage + delta_apparition, advance_percentage_bis);
            intensity = (intensity - 0.5f) * -1f + 0.5f; //Inverse the smoothstep function
        }
        intensity *= overall_alpha;

        vec4 pos = pos_line_straight + offset_dir * sin(phase[0] + 2.f * 3.14f * advance_percentage / 4.f) * 4.f;
        gl_Position = pos;
        gl_Position = projection * view * model * gl_Position;
        EmitVertex();

        gl_Position = vec4(pos.x, pos.y + 0.1f, pos.z, pos.w);
        gl_Position = projection * view * model * gl_Position;
        EmitVertex();

        pos_line_straight = pos_line_straight + vec4(dir_wind, 0.0f) * increment;
    }

    EndPrimitive();
}
