#version 450

out vec4 FragColor;
in float distToCamera;

void main()
{
        vec3 colFar = vec3(84.0f / 255.0f, 124.0f / 255.0f, 252.0f / 255.0f);
        vec3 colClose = vec3(10.0f / 255.0f, 98.0f / 255.0f, 224.0f / 255.0f);
        vec3 colInter = mix(colClose, colFar, 0.5);

        int distFar2 = 400;
        int distFar1 = 300;

        if (distToCamera > distFar2)
        {
                FragColor = vec4(colFar, 1.0f);
        }
        else if (distToCamera > distFar1)
        {
                float fac = (distToCamera - distFar1) / (distFar2 - distFar1);
                fac = clamp(fac, 0, 1);
                FragColor = vec4(mix(colInter, colFar, fac), 1.0f);
        }
        else
        {
                float fac = distToCamera / distFar1;
                fac = clamp(fac, 0, 1);
                FragColor = vec4(mix(colClose, colInter, fac), 1.0f);
        }
}