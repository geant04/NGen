#version 330 core
out vec4 FragColor;

in vec3 fs_texCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, fs_texCoords);
}