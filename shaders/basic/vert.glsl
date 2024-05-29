#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nor;
layout (location = 2) in vec2 tex;

out vec3 fs_nor;
out vec3 fs_pos;
out vec2 fs_UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 modelInvTrans;

void main() 
{
    gl_Position = projection * view * model * vec4(pos, 1.0);

    mat3 invTranspose = mat3(modelInvTrans);
    fs_nor = normalize(invTranspose * nor);

    fs_pos = vec3(model * vec4(pos, 1.0));

    fs_UV = tex;
}