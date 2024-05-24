#version 330 core

in vec3 fs_Pos;
out vec4 out_Col;
uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;

// "random" number generator
float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
vec2 Hammersley(uint i, uint N) {
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 xi, vec3 n, float r) {
    float a = r * r;

    // convert the 2d sample into spherical coordinates
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sinTheta = sqrt(1.0- cosTheta * cosTheta);

    vec3 wh;
    wh.x = cos(phi) * sinTheta;
    wh.y = sin(phi) * sinTheta;
    wh.z = cosTheta;

    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);

    vec3 whW = tangent * wh.x + bitangent * wh.y + n * wh.z;
    return normalize(whW);
}

float DistributionGGX(vec3 n, vec3 w_h, float roughness) {
    float alpha = roughness * roughness;
    float a2 = alpha * alpha;

    float NdotH = max(dot(n, w_h), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = denom * denom * PI;
    return (nom / denom);
}

void main() {
    // TODO
    vec3 nor = normalize(fs_Pos);
    vec3 wo = nor;

    vec3 li = vec3(0.0);
    float totalWeight = 0.0;

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i) {
        vec2 xi = Hammersley(i, SAMPLE_COUNT);
        vec3 wh = ImportanceSampleGGX(xi, nor, roughness);
        vec3 wi = normalize(2.0 * dot(wo, wh) * wh - wo);

        float NdotL = max(dot(nor, wi), 0.0);
        if (NdotL > 0.0) {
            float D = DistributionGGX(nor, wh, roughness);
            float nDotwh = max(dot(nor, wh), 0.0);
            float whDotwo = max(dot(wh, wo), 0.0);
            float pdf = (D * nDotwh / (4.0 * whDotwo)) + 0.0001;

            float resolution = 1024.0;
            float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            li += textureLod(environmentMap, wi, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    li = li / totalWeight;

    out_Col = vec4(li, 1.0);
}
