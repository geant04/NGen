#version 330 core
out vec4 FragColor;

uniform vec3 u_CamPos;
uniform samplerCube envMap;

in vec3 fs_nor;
in vec3 fs_pos;

void main()
{
    vec3 albedo = vec3(0.9, 0.9, 0.9);
    vec3 lightPos = vec3(0.0, 5.0, 5.0);
    vec3 diffuse_li = vec3(1.0, 1.0, 1.0);
    vec3 specular_li = vec3(1.0, 1.0, 1.0);

    vec3 normal = normalize(fs_nor);
    vec3 lightDir = normalize(lightPos - fs_pos);
    vec3 wo = normalize(u_CamPos - fs_pos);
    vec3 wi = normalize(reflect(-wo, normal));

    vec3 reflectedColor = texture(envMap, wi).xyz;
    albedo *= reflectedColor;

    // valve's half-lambert approximation, not physically correct
    float lambert = dot(normal, lightDir) * 0.50 + 0.50;

    //lambert *
    vec3 diffuse_lo = diffuse_li;

    // phong model
    vec3 h = normalize(lightDir + wo);
    float spec = pow(max(dot(normal, h), 0.0), 32.0);
    vec3 specular_lo = specular_li * spec * 1.0;

    vec3 outColor = diffuse_lo * albedo + specular_lo;
    //outColor *= albedo;

    FragColor = vec4(outColor, 1.0);
}