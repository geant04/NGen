#version 330 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 cameraPos;

out vec4 outColor;
in vec2 fs_UV;

const float resolution = 0.5;
const float stride = 1.0;

uniform float offset;
uniform int steps;

uniform float maxDistance;
uniform float thickness;

uniform float near;
uniform float far;

uniform bool binSearch;
uniform bool visCheck;

vec3 viewToFrag(vec4 view)
{
    vec4 frag = projection * view;
    frag.xy /= frag.w;
    frag.xy = frag.xy * 0.5 + 0.5; // [-1, 1] to [0, 1]

    return frag.xyz;
}

float linearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near)); // divide maybe not necessary for precision
}

bool screenSpaceRayMarch(
    vec3 fragStart, 
    vec3 fragEnd,
    out vec2 hit,
    out vec2 binaryT,
    out float hitDepth)
{       
    float prevT;
    float fragDepth;

    for (int i = 0; i < steps; i++)
    {
        float t = float(i) / float(steps);
        vec3 frag = mix(fragStart, fragEnd, t);

        if (frag.x > 1 || frag.x < 0 || frag.y > 1 || frag.y < 0)
        {
            return false;
        }

        float depth = (projection * view * texture2D(gPosition, frag.xy)).z;
        fragDepth = 1.0 / (mix(1.0 / fragStart.z, 1.0 / fragEnd.z, t) + 0.001);

        float diff = fragDepth - depth;

        if (diff > 0 && diff < thickness)
        {
            hit = frag.xy;
            hitDepth = fragDepth;
            binaryT = vec2(t, prevT);
            return true;
        } else
        {
            prevT = t;
        }
    }

    return false;
}

void binarySearch(vec3 fragStart, vec3 fragEnd, vec2 binaryT, out vec2 hit, out float hitDepth)
{
    float t = binaryT.x;
    float prevT = binaryT.y;
    t = prevT + ((t - prevT) / 2.0);

    for (int i = 0; i < 10; i++)
    {
        vec3 frag = mix(fragStart, fragEnd, t);
        if (frag.x > 1 || frag.x < 0 || frag.y > 1 || frag.y < 0)
        {
            break;
        }

        float depth = (projection * view * texture2D(gPosition, frag.xy)).z;
        float fragDepth = 1.0 / (mix(1.0 / fragStart.z, 1.0 / fragEnd.z, t) + 0.001);
        hitDepth = fragDepth;

        float diff = fragDepth - depth;

        if (diff > 0 && diff < thickness) // hit (overshot), move left
        {
            t = prevT + ((t - prevT) / 2.0);
        } else // miss, go right
        {
            float temp = t;
            t = t + ((t - prevT) / 2.0);
            prevT = temp;
        }

        hit = mix(fragStart, fragEnd, t).xy;
    }
}

void visibilityCheck(vec2 uv, out float alpha, float dotWoWi, float depth)
{
    alpha = alpha
        * (1.0 - dotWoWi)
        * (1.0 - clamp(depth / thickness, 0, 1))
        * (1.0 - smoothstep(0.95, 1.0, uv.y))
        * smoothstep(0, 0.05, uv.y)
        * smoothstep(0, 0.05, uv.x)
        * (1.0 - smoothstep(0.95, 1.0, uv.x));
}

void main()
{    
    vec3 worldPos = texture2D(gPosition, fs_UV.xy).xyz;
    vec3 worldNor = texture2D(gNormal, fs_UV.xy).xyz;

    // need to perform SSR; return UV coordinate of reflected hit
    vec3 viewDir = normalize(worldPos - cameraPos);
    vec3 reflectDir = normalize(reflect(viewDir, worldNor)); // simplify life by just doing this in world space first
    
    // offset by an epsilon
    vec4 startView = vec4(worldPos.xyz + worldNor * offset, 1.0);
    vec4 endView = vec4(worldPos.xyz + reflectDir * maxDistance, 1.0);

    startView = view * startView;
    endView = view * endView;

    // perform raymarch from startView to endView for atmost n steps
    // if we encounter a hit, return the UV
    //vec4 mainSceneColor = texture2D(gAlbedo, vec2(0.));
    float depth = texture2D(gMaterial, fs_UV.xy).z;
    float alpha = 1.0;
    vec2 hitUV;
    vec2 binaryT;
    
    vec3 fragStart = viewToFrag(startView);
    vec3 fragEnd = viewToFrag(endView);

    bool hit = screenSpaceRayMarch(fragStart, fragEnd, hitUV, binaryT, depth);
    if (hit && binSearch) binarySearch(fragStart, fragEnd, hitUV, binaryT, depth);
    if (hit && visCheck) visibilityCheck(hitUV, alpha, max(dot(-viewDir, reflectDir), 0.0), depth);
    // binarySearch refinement if hit
    vec3 color = (hit ? 1 : 0) * texture2D(gAlbedo, hitUV).rgb;

    outColor = vec4(color, 1.0);
}