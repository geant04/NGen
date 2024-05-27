#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gMaterial;

// fs_Pos should be in world coordinates, so view * Proj * (model * vec4(aPos));
in vec3 fs_Pos;
in vec3 fs_Nor;
in vec2 fs_UV;

// uniforms
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform float u_roughness;
uniform float u_metallic;
uniform vec3 u_albedo;

uniform bool useAlbedoMap;

// Gbuffer attachments:
// -- position buffer
// -- normal buffer
// -- albedo buffer
// depth is stored? but can that not be sampled from the z component of the position buffer?
// material buffer, which is different from the albedo buffer, like I think Mally has something like that

// Material buffer: 
// -- r: Metallic
// -- g: Roughness
// -- b: ?? who knows who cares

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, fs_UV).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_Pos);
    vec3 Q2  = dFdy(fs_Pos);
    vec2 st1 = dFdx(fs_UV);
    vec2 st2 = dFdy(fs_UV);

    vec3 N   = normalize(fs_Nor);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    vec3 albedo = u_albedo;
    if (useAlbedoMap) {
        albedo = texture(albedoMap, fs_UV).rgb;
    }

    gPosition = vec4(fs_Pos, 1.0);
    gNormal = vec4(getNormalFromMap(), 1.0);
    gAlbedo = vec4(albedo.rgb, 1.0);

    float metalMapValue = texture(metallicMap, fs_UV).r;
    float roughMapValue = texture(roughnessMap, fs_UV).r;

    gMaterial = vec4(metalMapValue, roughMapValue, 1.0, u_metallic);
}