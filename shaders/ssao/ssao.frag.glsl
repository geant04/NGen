#version 330 core

out vec2 outColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform int samples;
uniform mat4 projection;
uniform mat4 view;

in vec2 fs_UV;

const float PI = 3.14159f;
uniform float radius;
uniform float aoStrength;

float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    // 
    vec3 worldPos = texture(gPosition, fs_UV).rgb;
    vec3 normal = normalize(texture(gNormal, fs_UV).rgb);
    vec3 viewPos = (view * vec4(worldPos, 1.0)).rgb;
    
    float ao = 0;
    float thickness = 0;

    ivec2 coord = ivec2(gl_FragCoord.xy);
    float seed = hash(vec2(coord.x, coord.y));

    // build TBN
    vec3 tangent = vec3(0, 1, 0);
    vec3 bitangent = normalize(cross(tangent, normal));
    tangent = normalize(cross(normal, bitangent));

    mat3 TBN = mat3(tangent, bitangent, normal);

    for (int i = 0; i < samples; i++) {
        
        // generate random sample within hemisphere
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

        // create a tangent sample vector, ao sampling
        vec3 sample = TBN * vec3(x, y, z);
        vec3 offsetPos = worldPos + sample * radius;

        vec4 offset = projection * view * vec4(offsetPos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        vec4 sampleWorld = vec4(texture(gPosition, offset.xy).rgb, 1.0);
        vec4 sampleToPixel = projection * view * sampleWorld;
        sampleToPixel.xyz /= sampleToPixel.w;

        float sampleZ = sampleToPixel.z * 0.5 + 0.5;
        
        if (offset.z > sampleZ && length(worldPos - sampleWorld.rgb) < radius * 2.0)
        {
            ao += 1.0;
        }

        // thickness part, sample * radius should invert the hemisphere
        offsetPos = worldPos - sample * radius;
        offset = projection * view * vec4(offsetPos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        sampleWorld = vec4(texture(gPosition, offset.xy).rgb, 1.0);
        sampleToPixel = projection * view * sampleWorld;
        sampleToPixel.xyz /= sampleToPixel.w;

        sampleZ = sampleToPixel.z * 0.5 + 0.5;
        if (offset.z < sampleZ && length(worldPos - sampleWorld.rgb) < radius * 2.0)
        {
            thickness += 1.0;
        }
    }

    ao = 1.0 - (ao / float(samples));
    ao = clamp(pow(ao, aoStrength), 0, 1);

    thickness = (thickness / float(samples));
    
    outColor = vec2(ao, thickness);
}