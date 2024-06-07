#version 330 core

uniform sampler2D sampleTexture;
uniform sampler2D kernel;
uniform int kernelRadius;
uniform int u_PingPong;
uniform bool isAO;

in vec2 fs_UV;
out vec4 out_Col;

void aoFlip(inout vec4 color, in bool isAO) {
    if (isAO) {
        color.r = 1.0 - color.r;
    }
}

void main() {
    // TODO: Apply a Gaussian blur to the screen-space reflection
    // texture using the kernel stored in kernel.
    vec2 texSize = textureSize(sampleTexture, 0);
    float visibility = texture(sampleTexture, fs_UV).a;

    vec4 fragColor = texture(sampleTexture, fs_UV);
    float initWeight = texture(kernel, vec2(0.50, 1.0)).r;
    float boolAO = float(isAO);
    aoFlip(fragColor, isAO);

    fragColor *= initWeight;

    int bilinearRadius = kernelRadius / 2;

    for (int i = 1; i < bilinearRadius; i++) {
        float sampleIndex = 0.50 - (i / float(kernelRadius));
        float weight = texture(kernel, vec2(sampleIndex, 1.0)).r;

        vec2 dir = vec2(0., (float(i) / texSize.x));
        if (u_PingPong == 1) {
            dir = vec2((float(i) / texSize.y), 0.);
        }

        vec4 color = texture(sampleTexture, fs_UV + dir);
        aoFlip(color, isAO);

        fragColor += color * weight;

        color = texture(sampleTexture, fs_UV - dir);
        aoFlip(color, isAO);

        fragColor += color * weight;
    }

    aoFlip(fragColor, isAO);
    out_Col = fragColor;
}