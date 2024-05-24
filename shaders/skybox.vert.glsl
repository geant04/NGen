#version 330 core
layout (location = 0) in vec3 vs_Pos;
layout (location = 1) in vec2 vs_UV;
layout (location = 2) in vec3 vs_Nor;

out vec3 fs_Pos;
out vec3 fs_Nor;
out vec2 fs_UV;

uniform mat4 projection;
uniform mat4 view;

void main() 
{
    vec4 pos = projection * view * vec4(vs_Pos, 1.0);
    gl_Position = pos.xyww;
    fs_Pos = vs_Pos;
    fs_Nor = vs_Nor;
    fs_UV = vs_UV;
}