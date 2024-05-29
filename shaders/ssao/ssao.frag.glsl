#version 330 core

out float outColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform int samples;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 camPos;

in vec2 fs_UV;

const float PI = 3.14159f;
uniform float bias = 0.025;
uniform float radius = 0.14;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // 
    vec3 worldPos = texture(gPosition, fs_UV).rgb;
    vec3 normal = normalize(texture(gNormal, fs_UV).rgb);
    float fragPosZ = length(camPos - worldPos);
    
    float ao = 0;

    ivec2 coord = ivec2(gl_FragCoord.xy);
    float seed = hash(vec2(coord.x, coord.y));

    // build TBN
    vec3 tangent = vec3(0, 1, 0);
    vec3 bitangent = normalize(cross(tangent, normal));
    tangent = normalize(cross(normal, bitangent));

    // generate random samples [-1, 1]
    for (int i = 0; i < samples; i++) {
        // Generate random sample within hemisphere
        float rand = hash(vec2(seed, i));
        float rand2 = hash(vec2(i, rand));

        float hx = hash(vec2(rand + i, rand2));
        float hy = hash(vec2(rand2, rand + i));

        // cosine weighted sampling for AO
        float r = sqrt(hx);
        float theta = 2.0 * PI * hy;

        float x = r * cos(theta);
        float y = r * sin(theta);
        float z = sqrt(1.0 - hx);

        // create a tangent sample vector
        vec3 sampleVec = x * bitangent + y * tangent + z * normal;
        vec3 samplePos = worldPos + sampleVec * radius;
        vec4 viewPos = view * vec4(samplePos, 1.0);

        vec4 offset = projection * viewPos;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        // depth check, probably not the best approach
        vec4 sampleWorld = texture(gPosition, offset.xy);
        float sampleDepth = length(camPos - samplePos);
        float deferredDepth = length(camPos - sampleWorld.rgb);

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPosZ - sampleDepth));
        ao += (sampleDepth >= deferredDepth + bias ? 1.0 : 0.0) * rangeCheck;
    }

    ao = 1.0 - (ao / float(samples));

    outColor = ao;
}