#version 450
layout (points) in;
//layout (points, max_vertices=256) out;
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

    vec4 pos = gl_in[0].gl_Position;
    vec4 original_pos = pos;

    // polynomial equal to 0 at 0, 1 at 0.5, 0 at 1
    float overall_alpha = -4.f * animation_percentage * animation_percentage + 4.f * animation_percentage;

    int max_rectangles = 102;
    float size_wind = 50.f;
    float increment_x = size_wind / max_rectangles;
    vec4 dir_increment = vec4(vec3(dir[0] * increment_x), 0.0f);
    vec4 offset_dir = vec4(normalize(cross(dir[0], vec3(0.0f, 0.0f, 1.0f))), 0.0f);

    float delta_apparition = 0.3f;
    for (int i = 0; i < max_rectangles; ++i) {
        float advance_percentage = float(i) / float(max_rectangles);
        advance_percentage = advance_percentage * (1.0f + 2 * delta_apparition) - delta_apparition;
        // Advance_percentage goes from -delta_apparition to 1 + delta_apparition
        // It serves to regulate how the opcaity of a point on the line appears
        if (advance_percentage < animation_percentage)
            intensity = smoothstep(animation_percentage - delta_apparition, animation_percentage - 0.2f, advance_percentage);
        else {
            intensity = smoothstep(animation_percentage + 0.2f, animation_percentage + delta_apparition, advance_percentage);
            intensity = (intensity - 0.5f) * -1f + 0.5f; //Inverse the smoothstep function
        }
        intensity *= overall_alpha;

        gl_Position = pos;
        gl_Position = projection * view * model * gl_Position;
        EmitVertex();

        gl_Position = vec4(pos.x, pos.y + 0.1f, pos.z, pos.w);
        gl_Position = projection * view * model * gl_Position;
        EmitVertex();

        pos = pos + dir_increment;
        pos = pos + offset_dir * sin(phase[0] + 2.f * 3.14f * advance_percentage / 4.f) / 5.f;
        //pos = vec4(pos.x + dir_increment.x, original_pos.y + sin(2.f * 3.14f * advance_percentage / 1.f) / 2.f, pos.z + dir_increment.z, pos.w);
        //pos = vec4(pos.x + increment_x, original_pos.y + sin(2.f * 3.14f * advance_percentage / 1.f) / 5.f, pos.z, pos.w);
    }

    EndPrimitive();
}
