#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nor;
layout (location = 2) in vec2 tex;

out vec3 fs_Nor;
out vec3 fs_Pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() 
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
    fs_Nor = nor;
    fs_Pos = vec3(model * vec4(pos, 1.0));
}