#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

out vec2 fs_UV;
out vec3 fs_Pos;

void main() {
    gl_Position = vec4(pos, 1.0);
    fs_UV = texCoord;
}