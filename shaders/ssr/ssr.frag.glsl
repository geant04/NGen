#version 330 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 cameraPos;

out vec4 outColor;
in vec2 fs_UV;

const float resolution = 0.5;
const float maxDistance = 10.0;
const int steps = 10;
const float thickness = 0.0015;

const float near = 0.1;
const float far = 1000.0;

vec3 viewToFrag(vec4 view)
{
    vec4 frag = projection * view;
    frag.xyz /= frag.w; // perspective divide... purposefully incorrect without dividing z for now
    frag.xy = frag.xy * 0.5 + 0.5; // [-1, 1] to [0, 1]

    vec2 texSize = textureSize(gPosition, 0).xy;
    frag.xy = frag.xy * texSize;
    return frag.xyz;
}

// z is [-1, 1], not [0, 1]
float linearizeDepth(float z)
{
    return (2.0 * near * far) / (far + near - z * (far - near));
}

// return UV, 0,1 for binaryHit, depth
vec4 raymarch(vec4 startView, vec4 endView)
{
    // convert startView and endView to screenspace [-1, 1] coordinates via projection matrix multiplication
    // march along screenspace, interpolating between marched depth vs detected depth (use converted depth spaces to compare)
    vec2 texSize = textureSize(gPosition, 0).xy;

    vec3 startFrag = viewToFrag(startView);
    vec3 endFrag = viewToFrag(endView);

    startFrag.z = linearizeDepth(startFrag.z);
    endFrag.z = linearizeDepth(startFrag.z);

    float dx = endFrag.x - startFrag.x;
    float dy = endFrag.y - startFrag.y;

    float useX = abs(dx) >= abs(dy) ? 1 : 0;
    float delta = mix(abs(dy), abs(dx), useX) * clamp(resolution, 0, 1); // chooses btwn dy or dx
    vec2 increment = vec2(dx, dy) / max(delta, 0.001);

    vec2 frag = startFrag.xy;

    int hit0 = 0;
    int hit1 = 0;
    float t = 0;
    float prev = 0;
    float depth = 0;

    //raw search first 
    for (int i = 0; i < int(delta); i++)
    {
        if (frag.x > texSize.x || frag.y > texSize.y || frag.x < 0 || frag.y < 0) break;
        frag += increment;

        vec4 sampledDepth = texture2D(gPosition, frag / texSize);
        sampledDepth = projection * view * vec4(sampledDepth.xyz, 1.0);
        sampledDepth.xyz /= sampledDepth.w;
        sampledDepth.z = linearizeDepth(sampledDepth.z);

        // gather marched depth
        t = length(frag - startFrag.xy) / length(endFrag.xy - startFrag.xy);
        depth = mix(startFrag.z, endFrag.z, t);
        depth = depth - sampledDepth.z;

        if (depth > 0 && depth < thickness && sampledDepth.a > 0)
        {
            hit0 = 1;
            break;
        }
        else
        {
            prev = t;
        }
    }    

    t = prev + (t - prev) / 2.0; // t = half of previous t, perform a binary search

    for (int i = 0; i < steps * hit0; i++)
    {
        if (frag.x > texSize.x || frag.y > texSize.y || frag.x < 0 || frag.y < 0) break;
        frag = mix(startFrag.xy, endFrag.xy, t);

        vec4 sampledDepth = texture2D(gPosition, frag / texSize);
        sampledDepth = projection * view * vec4(sampledDepth.xyz, 1.0);
        sampledDepth.xyz /= sampledDepth.w;
        sampledDepth.z = linearizeDepth(sampledDepth.z);

        //depth = (startFrag.z * endFrag.z) / mix(endFrag.z, startFrag.z, t); // perspective-correct interpolation
        depth = mix(startFrag.z, endFrag.z, t);
        depth = depth - sampledDepth.z;

        if (depth > 0 && depth < thickness && sampledDepth.a > 0)
        {
            hit1 = 1;
            t = prev + (t - prev) / 2.0; // left half
        }
        else
        {
            float tempT = t;
            t = t + (t - prev) / 2.0;
            prev = tempT;
        }
    }

    frag.xy /= texSize;
    frag.x = clamp(frag.x, 0, 1);
    frag.y = clamp(frag.y, 0, 1);

    return vec4(frag.xy, hit1, depth);
}

float getVisibility(vec2 uv, float hit1, float depth, float lambert)
{
    vec3 worldFragPos = texture(gPosition, uv).xyz;
    vec3 worldPos = texture(gPosition, fs_UV).xyz;

    float visibility = hit1
            // * texture(u_TexMetalRoughMask, uv).b
            * (1 - lambert)
            * (1 - clamp(depth / thickness, 0, 1))
            * (1 - clamp(distance(worldFragPos, worldPos) / maxDistance, 0, 1))
            * (1 - smoothstep(0.95, 1.0, uv.y))
            * smoothstep(0, 0.05, uv.y)
            * smoothstep(0, 0.05, uv.x)
            * (1 - smoothstep(0.95, 1.0, uv.x))
            ;

    visibility = clamp(visibility, 0, 1);

    return visibility;
}

void main()
{
    vec3 worldPos = texture2D(gPosition, fs_UV.xy).xyz;
    vec3 worldNor = texture2D(gNormal, fs_UV.xy).xyz;

    // need to perform SSR; return UV coordinate of reflected hit
    vec3 viewDir = normalize(worldPos - cameraPos);
    vec3 reflectDir = normalize(reflect(worldNor, viewDir)); // simplify life by just doing this in world space first
    
    // offset by an epsilon
    vec4 startView = vec4(worldPos.xyz + (worldNor * 0.05), 1.0);
    vec4 endView = vec4(worldPos.xyz + reflectDir * maxDistance, 1.0);

    startView = view * startView;
    endView = view * endView;

    // perform raymarch from startView to endView for atmost n steps
    // if we encounter a hit, return the UV
    vec4 marchResults = raymarch(startView, endView);
    vec2 reflectedUV = marchResults.xy;

    // visibility check
    float lambert = max(dot(viewDir, reflectDir), 0);
    float visibility = getVisibility(reflectedUV, marchResults.z, marchResults.w, lambert);

    vec4 proj = projection * startView;
    proj.xyz /= proj.w;
    float dep = linearizeDepth(proj.z);

    vec4 mainSceneColor = texture2D(gAlbedo, reflectedUV);
    outColor = vec4(vec3(dep) + mainSceneColor.rgb * 0.000001, 1.0);
}