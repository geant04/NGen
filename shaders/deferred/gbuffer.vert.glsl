#version 330 core

layout (location = 0) in vec3 vs_Pos;
layout (location = 1) in vec3 vs_Nor;
layout (location = 2) in vec2 vs_UV;

out vec3 fs_Pos;
out vec3 fs_Nor;
out vec2 fs_UV;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec4 modelPos = model * vec4(vs_Pos, 1.0);
    gl_Position = projection * view * modelPos;

    mat3 invTranspose = transpose(inverse(mat3(model)));

    fs_Pos = modelPos.xyz;
    fs_Nor = normalize(invTranspose * vs_Nor);
    fs_UV = vs_UV;
}
