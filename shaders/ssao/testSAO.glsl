#version 330 core

out vec4 FragColor;

in vec2 fs_UV;

uniform sampler2D testTXT;

void main()
{
    vec3 ao = texture(testTXT, fs_UV).rgb;
    FragColor = vec4((ao), 1.0);
}