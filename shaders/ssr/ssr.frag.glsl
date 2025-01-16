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

uniform float near;
uniform float far;

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
    vec3 fragStart = viewToFrag(startView);
    vec3 fragEnd = viewToFrag(endView);

    fragStart.z = linearizeDepth(fragStart.z);
    fragEnd.z = linearizeDepth(fragEnd.z);

    for (int i = 0; i < steps; i++)
    {
        float t = float(i) / float(steps);
        vec3 frag = mix(fragStart, fragEnd, t);

        float depth = linearizeDepth(texture2D(gMaterial, frag.xy).z);
        float diff = frag.z - depth;

        if (diff >= 0 && diff < thickness)
        {
            hit = frag.xy;
            return true;
        }
    }

    return false;
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