#version 330 core
out vec4 out_Col;

uniform vec3 u_CamPos;

uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D gMaterial;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_SpecularMap;
uniform sampler2D u_BRDFLUT;
uniform sampler2D u_SSAO;

uniform bool u_DebugSSAO;
uniform bool u_EnableSSAO;

uniform float aoVal;

in vec2 fs_UV;

const vec3 light_pos[4] = vec3[](vec3(-10, 10, -10),
                                 vec3(10, 10, -10),
                                 vec3(-10, -10, -10),
                                 vec3(10, -10, -10));

const vec3 light_col[4] = vec3[](vec3(300.f, 300.f, 300.f) * 4,
                                 vec3(0.f, 300.f, 300.f) * 1,
                                 vec3(300.f, 0.f, 300.f) * 1,
                                 vec3(300.f, 300.f, 0.f) * 1);

const float PI = 3.14159f;

float distribFunc(vec3 n, vec3 wh, float roughness) 
{
    float alpha = roughness * roughness;
    float a2 = alpha * alpha;

    float NdotH = max(dot(n, wh), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = denom * denom * PI;
    return (nom / denom);
}

float gSub(float NdotV, float k) 
{
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    return nom / denom;
}

float geometricAtten(vec3 n, vec3 v, vec3 l, float k) 
{
    float NdotV = max(dot(n, v), 0.0);
    float NdotL = max(dot(n, l), 0.0);

    float gNV = gSub(NdotV, k);
    float gNL = gSub(NdotL, k);

    return gNV * gNL;
}

vec3 fresnelSchlick(float theta, vec3 R, float roughness) 
{
    //return R + (1.0 - R) * pow(1.0 - theta, 5.0);
    return R + (max(vec3(1.f - roughness), R) - R) * pow(1.f - theta, 5.0f);
}

vec3 glint(vec3 wh, float targetNDF, float maxNDF, vec2 uv, vec2 duvdx, vec2 duvdy) {
    // modify the NDF to be matching of the glint ndf
    // sample a random theta, used for our bernoulli trials
    // temporal coherence: this is like basically going from one LOD to another LOD
    // solving anisotrphy by also doing some axis-aligning UV tricks
    // the paper explains it all, let's just figure out a way to set up the ...
    // set up the random texture our UVs will sample from

    return vec3(0.);
}

const float DISTORTION = 0.9;
const float SCALE = 9.0;
const float AMBIENT = 0.9;
const float GLOW = 1.0;

vec3 subsurfaceColor(vec3 lightDir, vec3 normal, vec3 viewVec, float thin, vec3 albedo, vec3 lightColor, float glow) {
    vec3 scatterDir = lightDir + normal * DISTORTION;
    float lightReachingEye = pow(clamp(dot(viewVec, -scatterDir),
                                       0.0, 1.0), glow) * SCALE;
    float attenuation = max(0.0, dot(normal, lightDir)
                            + dot(viewVec, -lightDir));
    float totalLight = attenuation * (lightReachingEye + AMBIENT) * thin;
    return albedo * lightColor * totalLight;
}

void main()
{
    vec3 albedo = pow(texture(gAlbedo, fs_UV).rgb, vec3(2.2));
    vec4 material = texture(gMaterial, fs_UV);
    float roughness = material.g;
    float metallic = material.r;
    vec3 normal = texture(gNormal, fs_UV).rgb;
    float ao = aoVal;
    float thickness = 0.0;

    if (u_EnableSSAO)
    {
        // testing thickness
        vec2 aoProps = texture(u_SSAO, fs_UV).rg;
        ao *= aoProps.r;
        thickness = aoProps.g;
    }
    if (u_DebugSSAO)
    {
        out_Col = vec4(vec3(ao), 1.0);
        return;
    }

    vec4 gPos = texture(gPosition, fs_UV);
    vec3 fs_Pos = gPos.rgb;

    vec3 Lo = vec3(0.);
    vec3 intensity = 0.03f * 1.0 * albedo;

    // use this to toggle the epic sparkles
    bool sparkly = false;

    bool pointLight = true;

    vec3 wo;
    vec3 wi;
    vec3 wh;

    // VARIABLES FOR TORRANCE-SPARROW
    float D;
    float G;
    vec3 F;

    // VARIABLES FOR KS FRESNEL
    vec3 f_0 = mix(vec3(0.04f), albedo, metallic);

    if (pointLight) {
        for (int i = 0; i < 4; i++) {
            vec3 diff = light_pos[i] - fs_Pos;
            vec3 light_color = light_col[i];
            vec3 irradiance = light_color / dot(diff, diff);

            wi = normalize(diff);
            wo = normalize(u_CamPos - fs_Pos);
            wh = normalize((wo + wi) / 2.0f);

            // distribution of facets ratio based on roughness
            D = distribFunc(normal, wh, roughness);

            // geometric attenuation (self occluding)
            float k = pow(roughness + 1.0f, 2.f) / 8.0f;
            G = geometricAtten(normal, wo, wi, k);

            // fresnel reflectance
            F = fresnelSchlick(max(dot(wh, wo), 0.0), f_0, roughness);
            vec3 ks = (D*G*F) / (4.0f * max(dot(normal, wo), 0.) * max(dot(normal, wi), 0.) + 0.0001f);

            vec3 kd = vec3(1.0f) - F;
            kd *= (1.0f - metallic);

            vec3 f_lambert = (albedo / PI);

            vec3 f = kd * f_lambert + ks;

            // irradiance is l_i, bsdf is bsdf, pdf = 1.0, and account for the lambert
            Lo += f * irradiance * max(dot(wi, normal), 0.);
        }
        Lo += intensity;
    }

    wo = normalize(u_CamPos - fs_Pos);
    wi = normalize(reflect(-wo, normal));
    wh = normalize(normal);

    F = fresnelSchlick(max(dot(wh, wo), 0.0), f_0, roughness);
    vec3 kS = F;

    // now to use our awesome diffuse irradiance map
    vec3 kD = 1 - kS;
    kD *= (1 - metallic);

    vec3 diffuse_li = texture(u_IrradianceMap, normal).rgb;
    vec3 diffuse_lo = diffuse_li * kD * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 specular_li = textureLod(u_SpecularMap, wi, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF = texture(u_BRDFLUT, vec2(max(dot(wh, wo), 0.0), roughness)).rg;
    vec3 specular_lo = specular_li * (F * envBRDF.x + envBRDF.y);

    Lo += diffuse_lo + specular_lo;
    Lo *= ao;

    // Subsurface Scattering
    vec3 ssr = subsurfaceColor(-wo, 
                                normal, 
                                wo, 
                                thickness, 
                                albedo, 
                                vec3(1, 0, 0) * diffuse_li, 
                                GLOW);
    // TO DO: ADD ADJUSTABLE TOGGLES FOR:
    // - GLOW
    // - DISTORTION
    // - SCALE
    // - AMBIENT
    // - LIGHT COLOR
    Lo += ssr;

    // Gamma correction
    Lo = Lo / (vec3(1.0f) + Lo);
    float gamma = 2.2f;
    Lo = pow(Lo, vec3(1.0 / gamma));

    // Render skybox logic
    float alpha = material.b;
    if (alpha != 1.0) {
        out_Col = vec4(vec3(1.), .0);
        return;
    }

    out_Col = vec4(Lo, alpha);
}