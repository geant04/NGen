#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nor;
layout (location = 2) in vec2 tex;

out vec3 fs_Nor;
out vec3 fs_Pos;
out vec2 fs_UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 modelInvTrans;

void main() 
{
    mat3 invTranspose = mat3(modelInvTrans);

    fs_Nor = normalize(invTranspose * nor);

    vec4 modelPos = model * vec4(pos, 1.0);
    fs_Pos = modelPos.xyz;

    gl_Position = projection * view * modelPos;

    fs_UV = tex;
}