#version 330 core
out vec4 FragColor;

uniform vec3 viewPos;

in vec3 fs_nor;
in vec3 fs_pos;

void main()
{
    vec3 albedo = vec3(0.5, 0.0, 0.0);
    vec3 lightPos = vec3(0.0, 5.0, 5.0);
    vec3 diffuse_li = vec3(1.0, 1.0, 1.0);
    vec3 specular_li = vec3(1.0, 1.0, 1.0);

    vec3 normal = normalize(fs_nor);
    vec3 wi = normalize(lightPos - fs_pos);
    vec3 wo = normalize(viewPos - fs_pos);

    // valve's half-lambert approximation, not physically correct
    float lambert = dot(normal, wi) * 0.50 + 0.50;

    vec3 diffuse_lo = lambert * diffuse_li;

    // phong model
    vec3 h = normalize(wi + wo);
    float spec = pow(max(dot(normal, h), 0.0), 32.0);
    vec3 specular_lo = specular_li * spec * 0.5;

    vec3 outColor = diffuse_lo * albedo + specular_lo;
    //outColor *= albedo;

    FragColor = vec4(outColor, 1.0);
}