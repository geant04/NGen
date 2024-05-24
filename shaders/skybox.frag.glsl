#version 330 core
out vec4 out_Col;

in vec3 fs_Pos;
in vec2 fs_UV;

uniform samplerCube skybox;
// this sampler is here for testing purposess
uniform sampler2D txt;

uniform bool u_isHDR;

void main()
{
    vec3 envColor = texture(skybox, fs_Pos).rgb;
    if (u_isHDR)
    {
        // gamma corrections
        envColor = envColor / (envColor + vec3(1.0));
        envColor = pow(envColor, vec3(1.0/2.2));
    }

    out_Col = vec4(envColor, 1.0);
}