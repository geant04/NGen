#version 330 core

layout (location = 0) in vec3 vs_Pos;

out vec3 fs_Pos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    fs_Pos = vs_Pos;
    gl_Position = projection * view * vec4(fs_Pos, 1.0);
}