#version 450
layout (points) in;
//layout (points, max_vertices=256) out;
layout (triangle_strip, max_vertices=1024) out;

out float intensity;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float animation_percentage; //0 for just starting, 1 for finished

void main() {

    vec4 pos = gl_in[0].gl_Position;
    vec4 original_pos = pos;

    // polynomial equal to 0 at 0, 1 at 0.5, 0 at 1
    float overall_alpha = -4.f * animation_percentage * animation_percentage + 4.f * animation_percentage;

    int max_rectangles = 102;
    float size_wind = 30.f;
    float increment_x = size_wind / max_rectangles;
    for (int i = 0; i < max_rectangles; ++i) {
        float advance_percentage = float(i) / float(max_rectangles);
        if (advance_percentage < animation_percentage)
            intensity = smoothstep(animation_percentage - 0.3f, animation_percentage - 0.2f, advance_percentage);
        else {
            intensity = smoothstep(animation_percentage + 0.2f, animation_percentage + 0.3f, advance_percentage);
            intensity = (intensity - 0.5f) * -1f + 0.5f;
        }
        intensity *= overall_alpha;
        //intensity = advance_percentage;
        /*if (advance_percentage >= animation_percentage - 0.2f && advance_percentage <= animation_percentage + 0.2f)
            intensity = 1.0f;
        else if
        */
        gl_Position = pos;
        gl_Position = projection * view * model * gl_Position;
        EmitVertex();

        gl_Position = vec4(pos.x, pos.y + 0.1f, pos.z, pos.w);
        gl_Position = projection * view * model * gl_Position;
        EmitVertex();

        pos = vec4(pos.x + increment_x, original_pos.y + sin(2.f * 3.14f * advance_percentage / 1.f) / 5.f, pos.z, pos.w);
    }

    EndPrimitive();
}
