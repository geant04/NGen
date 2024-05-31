#version 330 core

uniform sampler2D sampleTexture;
uniform sampler2D kernel;
uniform int kernelRadius;
uniform int u_PingPong;

in vec2 fs_UV;
out vec4 out_Col;

void main() {
    // TODO: Apply a Gaussian blur to the screen-space reflection
    // texture using the kernel stored in kernel.
    vec2 texSize = textureSize(sampleTexture, 0);
    float visibility = texture(sampleTexture, fs_UV).a;

    vec4 fragColor = texture(sampleTexture, fs_UV);
    float initWeight = texture(kernel, vec2(0.50, 1.0)).r;

    fragColor *= initWeight / 2.0;

    int bilinearRadius = kernelRadius / 2;

    for (int i = 1; i < bilinearRadius; i++) {
        float sampleIndex = 0.50 - (i / float(kernelRadius));
        float weight = texture(kernel, vec2(sampleIndex, 1.0)).r;

        vec2 dir = vec2(0., (float(i) / texSize.x));
        if (u_PingPong == 1) {
            dir = vec2((float(i) / texSize.y), 0.);
        }

        fragColor +=
            texture(sampleTexture, fs_UV + dir)
                 * weight;

        fragColor +=
            texture(sampleTexture, fs_UV - dir)
                 * weight;
    }

    out_Col = fragColor;
}