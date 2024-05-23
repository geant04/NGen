#version 330 core
out vec4 out_Col;
in vec3 fs_Pos;

// precompute irradiance map, this way we can do texture(diffuseMap, wi).rgb
// map itself represented the integral hemisphere for each texel on the cubemap

uniform samplerCube environmentMap;


const float PI = 3.14159265359;

void main()
{
    vec3 normal = normalize(fs_Pos);

    vec3 irradiance = vec3(0.);

    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));

    // from LearnOpenGL, the idea is to integrate over a hemisphere in N * N discrete samples
    // then accumulate the average color from the samples
    float sampleDelta = 0.025;
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    return vec4(irradiance, 1.0);
}