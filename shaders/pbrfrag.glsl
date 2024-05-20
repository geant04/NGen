#version 330 core
out vec4 out_Col;

uniform vec3 u_CamPos;
uniform vec3  u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AmbientOcclusion;

in vec3 fs_Nor;
in vec3 fs_Pos;
in vec2 fs_UV;

const vec3 light_pos[4] = vec3[](vec3(-10, 10, -10),
                                 vec3(10, 10, -10),
                                 vec3(-10, -10, -10),
                                 vec3(10, -10, -10));

// vec3(0, 10, 0) is a more dramatic angle ig
// vec3(0, 10, 5) is pretty decent

const vec3 light_col[4] = vec3[](vec3(300.f, 300.f, 300.f) * 4,
                                 vec3(300.f, 300.f, 300.f) * 1,
                                 vec3(300.f, 300.f, 300.f) * 1,
                                 vec3(300.f, 300.f, 300.f) * 1);

const float PI = 3.14159f;

float distribFunc(vec3 n, vec3 w_h, float roughness) {
    float alpha = roughness * roughness;
    float a2 = alpha * alpha;

    float NdotH = max(dot(n, w_h), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = denom * denom * PI;
    return (nom / denom);
}

float gSub(float NdotV, float k) {
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    return nom / denom;
}

float geometricAtten(vec3 n, vec3 v, vec3 l, float k) {
    float NdotV = max(dot(n, v), 0.0);
    float NdotL = max(dot(n, l), 0.0);

    float gNV = gSub(NdotV, k);
    float gNL = gSub(NdotL, k);

    return gNV * gNL;
}

vec3 fresnelSchlick(float theta, vec3 R) {
    //return R + (1.0 - R) * pow(1.0 - theta, 5.0);
    return R + (max(vec3(1.f - u_Roughness), R) - R) * pow(1.f - theta, 5.0f);
}

vec3 glint(vec3 w_h, float targetNDF, float maxNDF, vec2 uv, vec2 duvdx, vec2 duvdy) {
    // modify the NDF to be matching of the glint ndf
    // sample a random theta, used for our bernoulli trials
    // temporal coherence: this is like basically going from one LOD to another LOD
    // solving anisotrphy by also doing some axis-aligning UV tricks
    // the paper explains it all, let's just figure out a way to set up the ...
    // set up the random texture our UVs will sample from

    return vec3(0.);
}


void main()
{
    vec3 albedo = u_Albedo;
    float roughness = u_Roughness;
    float metallic = u_Metallic;

    vec3 Lo = vec3(0.);
    vec3 intensity = 0.03f * u_AmbientOcclusion * albedo;

    // use this to toggle the epic sparkles
    bool sparkly = false;

    for (int i = 0; i < 4; i++) {
        vec3 diff = light_pos[i] - fs_Pos;
        vec3 light_color = light_col[i];
        vec3 irradiance = light_color / dot(diff, diff);

        vec3 w_i = normalize(diff);
        vec3 w_o = normalize(u_CamPos - fs_Pos);
        vec3 w_h = normalize((w_o + w_i) / 2.0f);

        // distribution of facets ratio based on roughness
        float D = distribFunc(fs_Nor, w_h, roughness);

        // geometric attenuation (self occluding)
        float k = pow(roughness + 1.0f, 2.f) / 8.0f;
        float G = geometricAtten(fs_Nor, w_o, w_i, k);

        // fresnel reflectance
        vec3 f_0 = mix(vec3(0.04f), albedo, metallic);
        vec3 F = fresnelSchlick(max(dot(w_h, w_o), 0.0), f_0);

        vec3 f_cookTorrance = (D*G*F) / (4.0f * max(dot(fs_Nor, w_o), 0.) * max(dot(fs_Nor, w_i), 0.) + 0.0001f);

        // overall color should be kd * lamb + ks * cook_torrance -- note that ks = Fresnel
        // so in reality it should actually be kd(albedo/pi) + DFG/(4 dot (w_o, n) * dot(w_i, n))

        vec3 k_d = vec3(1.0f) - F;
        k_d *= (1.0f - metallic);

        vec3 f_lambert = (albedo / PI);

        vec3 f = k_d * f_lambert + f_cookTorrance;

        // irradiance is l_i, bsdf is bsdf, pdf = 1.0, and account for the lambert
        Lo += f * irradiance * max(dot(w_i, fs_Nor), 0.);
    }
    Lo += intensity;

    Lo = Lo / (vec3(1.0f) + Lo);
    float gamma = 2.2f;
    Lo = pow(Lo, vec3(1.0 / gamma));

    out_Col = vec4(Lo, 1.);
}