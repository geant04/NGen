#version 330 core
out vec4 out_Col;

in vec3 fs_Pos;

uniform samplerCube skybox;
uniform bool u_isHDR;

void main()
{
    out_Col = texture(skybox, fs_Pos);
    // if (u_isHDR)
    // {
    //     // gamma corrections
    //     envColor = envColor / (envColor + vec3(1.0));
    //     envColor = pow(envColor, vec3(1.0/2.2));
    // }

    // out_Col = vec4(envColor, 1.0);
}