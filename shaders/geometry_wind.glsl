#version 450
layout (points) in;
layout (triangle_strip, max_vertices=1024) out;

in vec3 dir[];
in int animated[];
in float anim_percentage[];
in float phase[];
in int spiral[];

out float intensity;

uniform vec3 camera_front;
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
    vec4 width_dir = vec4(0.f);
    vec4 pos;

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

        float x = advance_percentage;
        if (spiral[0] != 0) {
            float displacement = 1.f / (0.08f * sqrt(2.f * 3.14f)) * exp(-0.5f * pow((x - 0.24f) / 0.055f, 2.f));
            pos = pos_line_straight - offset_dir * displacement;
        } else {
            pos = pos_line_straight + offset_dir * sin(phase[0] + 2.f * 3.14f * advance_percentage / 3.f) * 4.f;
        }
        gl_Position = pos;
        gl_Position = projection * view * model * gl_Position;
        EmitVertex();

        gl_Position = pos + 0.1f * width_dir;
        //gl_Position = vec4(pos.x, pos.y + 0.1f, pos.z, pos.w);
        gl_Position = projection * view * model * gl_Position;
        EmitVertex();


        vec4 new_pos_line_straight;
        if (spiral[0] != 0)
            new_pos_line_straight = pos_line_straight + vec4(dir_wind, 0.0f) * (cos(4.f * 3.14f * x) + 0.5f * 2.f * 3.14f * x);
        else
            new_pos_line_straight = pos_line_straight + vec4(dir_wind, 0.0f) * increment;
        width_dir = vec4(normalize(cross(new_pos_line_straight.xyz - pos_line_straight.xyz, camera_front)), 0.0f);
        pos_line_straight = new_pos_line_straight;
    }

    EndPrimitive();
}
