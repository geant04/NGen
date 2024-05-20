#version 330 core
out vec4 out_Col;

uniform vec3 u_CamPos;
uniform vec3  u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AmbientOcclusion;

in vec3 fs_Nor;
in vec3 fs_Pos;

const vec3 light_pos[4] = vec3[](vec3(-10, 10, 10),
                                 vec3(10, 10, 10),
                                 vec3(-10, -10, 10),
                                 vec3(10, -10, 10));

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

float random3( vec3 p ) {
    return fract(sin((dot(p, vec3(127.1,
                                  311.7,
                                  191.999)))) *
                 43758.5453);
}

vec3 vec3pow(vec3 p, float t) {
    return vec3(pow(p.x, t), pow(p.y, t), pow(p.z, t));
}

float surflet(vec3 p, vec3 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    vec3 t2 = abs(p - gridPoint);

    vec3 t = vec3(1.f) - 6.f * vec3pow(t2, 5.f) + 15.f * vec3pow(t2, 4.f) - 10.f * vec3pow(t2, 3.f);
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a vec2 in the range [0, 1])
    vec3 gradient = random3(gridPoint) * 2. - vec3(1., 1., 1.);
    // Get the vector from the grid point to P
    vec3 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y * t.z;
}

float perlinNoise3D(vec3 p) {
        float surfletSum = 0.f;
        // Iterate over the four integer corners surrounding uv
        for(int dx = 0; dx <= 1; ++dx) {
                for(int dy = 0; dy <= 1; ++dy) {
                        for(int dz = 0; dz <= 1; ++dz) {
                                surfletSum += surflet(p, floor(p) + vec3(dx, dy, dz));
                        }
                }
        }
        return surfletSum;
}

vec3 glint(vec3 w_h, float targetNDF, float maxNDF, vec2 uv, vec2 duvdx, vec2 duvdy) {
    return vec3(0.);
}


void main()
{
    vec3 albedo = u_Albedo;
    float roughness = u_Roughness;
    float metallic = u_Metallic;

    vec3 Lo = vec3(0.);
    vec3 intensity = 0.03f * u_AmbientOcclusion * albedo;

    // use this to toggle the awesome mold shader
    bool getMoldy = false;

    // use this to toggle the epic sparkles
    bool sparkly = false;

    for (int i = 0; i < 4; i++) {
        vec3 diff = light_pos[i] - fs_Pos;
        vec3 light_color = light_col[i];
        vec3 irradiance = light_color / dot(diff, diff);

        vec3 w_i = normalize(diff);
        vec3 w_o = normalize(u_CamPos - fs_Pos);
        vec3 w_h = normalize((w_o + w_i) / 2.0f);

        if (getMoldy) {
            float moldIntensity = perlinNoise3D(fs_Pos * u_Roughness * 0.20) * 0.50 + 0.50;
            roughness = clamp(mix(1.25 * roughness, 1.0, moldIntensity), 0.50, 1.0);

            float freaky = u_Roughness * (perlinNoise3D(32.0 * fs_Pos / (moldIntensity + 0.001)));

            vec3 mold = 1.2 * mix(albedo, vec3(1.0), pow(moldIntensity, 1.0)) *
                    (perlinNoise3D(fs_Pos * pow(u_Roughness * 1.2, 2.0) * 2.0) + 1.0) * (freaky * 0.50 + 0.50);

            albedo = mix(albedo, mold, u_Roughness + freaky - 0.10);

            if (length(mold) > 0.40 && (u_Roughness + freaky > 0.70))  {
                albedo = mix(albedo, vec3(1.0), length(mold) );
                if (length(mold) > 1.25) {
                    albedo = vec3(1.0);
                }
            }

            float v = 1.2 * (perlinNoise3D(fs_Pos * 4.0) * 0.50 + 0.50);
            float splotchMask = 1.4 * (perlinNoise3D(fs_Pos * 1.0 + 2.20) * 0.50 + 0.50);
            splotchMask = smoothstep(0.40, 1.0, splotchMask);

            albedo += pow(u_Roughness, 6.0) * smoothstep(0.65, 1.0, v) * vec3(0.87);
            albedo *= vec3(1.0 - splotchMask * u_Roughness);

            roughness *= ((1.0 - splotchMask) * u_Roughness);
            metallic *= ((1.0 - splotchMask) * u_Roughness);

            if (length(albedo) < 0.24) {
                vec3 yuck = vec3(0.03, 0.09, 0.02);
                yuck = mix(yuck, vec3(1.0), length(albedo) * 1.5);
                albedo = mix(albedo, yuck, 0.60);
            }
        }

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