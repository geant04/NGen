#pragma once

#include "shader.h"
#include "blur.h"
#include "camera.h"
#include "mesh.h"

class SSAO
{
public:
    SSAO();
    ~SSAO();

    void Create(const unsigned int WIDTH, const unsigned int HEIGHT, const bool isHalf);
    void SSAOPass(const unsigned int gPosition, const unsigned int gNormal, Camera &camera, Mesh &quad);
    void SSAOBlurPass();
    void Clear();
    unsigned int GetSSAOBuffer() const { return SSAObuffer; }

    Shader GetShader() const { return SSAOShader; }
    Shader &GetShader() { return SSAOShader; }

    float SSAOradius;
    float SSAOstrength;
    float SSAOinvStrength;
    int SSAOsamples;
    unsigned int kernelRadius;

private:
    Shader SSAOShader;
    BlurFramebuffer SSAOBlur;
    unsigned int SSAObuffer, SSAOfbo;
    unsigned int WIDTH, HEIGHT;
    bool doBlur = true;
    bool isHalf;
};