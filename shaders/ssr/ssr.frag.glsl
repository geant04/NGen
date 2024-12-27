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
const int steps = 30;
const float stride = 1.0;

uniform float maxDistance;
uniform float thickness;

const float near = 1.0;
const float far = 1000.0;

vec3 viewToFrag(vec4 view)
{
    vec4 frag = projection * view;
    frag /= frag.w; // perspective divide... purposefully incorrect without dividing z for now
    frag.xy = frag.xy * 0.5 + 0.5; // [-1, 1] to [0, 1]

    return frag.xyz;
}

float linearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near)); // divide maybe not necessary for precision
}

bool screenSpaceRayMarch(
    vec4 startView, 
    vec4 endView,
    out vec2 hit,
    out float outDepth)
{   
    // homogenous clip space projection
    vec4 hClipStart = projection * startView;
    vec4 hClipEnd = projection * endView;

    float k0 = 1.0 / hClipStart.w;
    float k1 = 1.0 / hClipEnd.w;

    // camera space coordinates, but interpolated homogeneous
    vec3 q0 = hClipStart.xyz; 
    vec3 q1 = hClipEnd.xyz;
    
    // NDC screenspace coordinates [-1, 1] range I think... idk tbh
    vec2 p0 = hClipStart.xy * k0 * 0.5 + 0.5;
    vec2 p1 = hClipEnd.xy * k1 * 0.5 + 0.5;

    // do an unoptimized approach for now
    int stepCount = 64;
    int binSearch = 0;
    float increment = 1.0 / float(stepCount);
    float w = 0;
    float prevW = 0;

    for (int i = 0; i < stepCount; i++)
    {
        w += increment;

        float k = mix(k0, k1, w);
        vec3 q = mix(q0, q1, w);
        vec2 p = mix(p0, p1, w);

        if (p.x > 1 || p.y > 1 || p.x < 0 || p.y < 0) return false;

        float rayDepth = linearizeDepth(q.z * k);
        float sampleDepth = texture2D(gMaterial, p).z;
        sampleDepth = linearizeDepth(sampleDepth);

        float diff = rayDepth - sampleDepth;
        if (diff > 0 && diff < thickness)
        {
            hit = p;
            outDepth = rayDepth;
            binSearch = 1;
        }
        else
        {
            prevW = w;
        }
    }

    w = prevW + (w - prevW) / 2.0;
    int iterations = 10 * binSearch;
    
    for (int i = 0; i < iterations; i++)
    {
        float k = mix(k0, k1, w);
        vec3 q = mix(q0, q1, w);
        vec2 p = mix(p0, p1, w);

        if (p.x > 1 || p.y > 1 || p.x < 0 || p.y < 0) return false;

        float rayDepth = linearizeDepth(q.z * k);
        float sampleDepth = texture2D(gMaterial, p).z;
        sampleDepth = linearizeDepth(sampleDepth);

        float diff = rayDepth - sampleDepth;

        if (diff > 0 && diff < thickness)
        {
            hit = p;
            outDepth = rayDepth;
            w = prevW + (w - prevW) / 2.0;
        }
        else
        {
            float temp = w;
            w = w + (w - prevW) / 2.0;
            prevW = w;
        }
    }

    return false || (binSearch == 1);
}



void main()
{
    vec3 worldPos = texture2D(gPosition, fs_UV.xy).xyz;
    vec3 worldNor = texture2D(gNormal, fs_UV.xy).xyz;

    // need to perform SSR; return UV coordinate of reflected hit
    vec3 viewDir = normalize(worldPos - cameraPos);
    vec3 reflectDir = normalize(reflect(viewDir, worldNor)); // simplify life by just doing this in world space first
    
    // offset by an epsilon
    vec4 startView = vec4(worldPos.xyz, 1.0);
    vec4 endView = vec4(worldPos.xyz + reflectDir * maxDistance, 1.0);

    startView = view * startView;
    endView = view * endView;

    // perform raymarch from startView to endView for atmost n steps
    // if we encounter a hit, return the UV
    //vec4 mainSceneColor = texture2D(gAlbedo, vec2(0.));
    float depth = texture2D(gMaterial, fs_UV.xy).z;
    vec2 hitUV;
    float outDepth;
    bool hit = screenSpaceRayMarch(startView, endView, hitUV, outDepth);

    // binarySearch refinement if hit
    vec3 color = (hit ? 1 : 0) * texture2D(gAlbedo, hitUV).rgb;

    outColor = vec4(color, 1.0);
}