#version 330 core
layout (location = 0) in vec3 vs_Pos;

out vec3 fs_Pos;

uniform mat4 projection;
uniform mat4 view;

void main() 
{
    vec4 pos = projection * view * vec4(vs_Pos, 1.0);
    gl_Position = pos.xyww;
    fs_Pos = vs_Pos;
}