#version 450

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform DirLight dirLight;
uniform vec3 viewPosition;
uniform vec3 color;
uniform float zAtoon[256];
uniform sampler2D texture_diffuse1;
uniform bool use_color;
uniform float alpha_clip;
uniform bool use_zAtoon;
uniform bool no_texture;
out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 col);

void main() {
    vec3 col = color;

    if (use_color) {
        col = color;
    }
    else {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        if (texColor.a < alpha_clip) {
            discard;
        }
        col = vec3(texColor);
    }
    if (no_texture) {
        col = vec3(0.5f, 0.5f, 0.5f);
    }
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 result = CalcDirLight(dirLight, normal, viewDir, col);

    FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 col) {
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
    vec3 result = ambient + diffuse;

    return result;
}