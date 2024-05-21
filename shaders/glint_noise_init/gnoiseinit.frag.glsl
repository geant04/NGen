#version 330 core
out vec4 out_Col

in vec2 fs_UV;
in vec3 fs_Pos;



vec4 SampleGlints2023NDF(vec3 localH, float targetNDF, float maxNDF, vec2 uv, vec2 duvdx, duvdy)
{
    // BEGIN BY EXTRACING R AND THETA BY PROJECTING PIXEL FOOTPRINT
    
    // 
}

void main()
{
    out_Col = vec4(fs_UV, 0.0, 1.0);
}