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
out vec4 FragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main() {
    //Some computations are repetitive
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 result = CalcDirLight(dirLight, normal, viewDir);

    FragColor = vec4(result, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    //ambient
    vec3 ambient = light.ambient * color;

    // diffuse
    vec3 lightDirection = normalize(-light.direction);
    float coeff = max(dot(normal, lightDirection), 0.0f);
    coeff = zAtoon[int(coeff * 256.0f)];

    vec3 diffuse = light.diffuse * coeff * color;

    //specular
//    vec3 reflectDir = reflect(-lightDirection, normal);
//    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    vec3 specular = vec3(0.0f);

    //total
    vec3 result = ambient + diffuse + specular;

    return result;
}
