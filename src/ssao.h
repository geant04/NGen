#pragma once

#include "shader.h"
#include "blur.h"
#include "camera.h"
#include "mesh.h"

class SSAO
{
public:
    SSAO();
    SSAO(float radius, float strength, int samples, unsigned int kernelSize);
    ~SSAO();

    void Create(const unsigned int WIDTH, const unsigned int HEIGHT, const bool isHalf);
    void SSAOPass(const unsigned int gPosition, const unsigned int gNormal, Camera &camera, Mesh &quad);
    void SSAOBlurPass();
    void AssignParams(int samples, float radius, float strength, float sss_strength) {
        this->SSAOstrength = strength;
        this->SSAOradius = radius;
        this->SSAOsamples = samples;
        this->sss_strength = sss_strength;
    };
    void Clear();
    unsigned int GetSSAOBuffer() const { return SSAObuffer; }

    Shader GetShader() const { return SSAOShader; }
    Shader &GetShader() { return SSAOShader; }

    int* GetSamplesRef() { return &SSAOsamples; }
    float* GetRadiusRef() { return &SSAOradius; }
    float* GetStrengthRef() { return &SSAOstrength; }
    float* GetSubStrengthRef() { return &sss_strength; }

private:
    Shader SSAOShader;
    BlurFramebuffer SSAOBlur;
    unsigned int SSAObuffer, SSAOfbo;
    unsigned int WIDTH, HEIGHT;
    unsigned int kernelRadius;
    float SSAOradius, SSAOstrength, sss_strength;
    int SSAOsamples;
    bool doBlur = true;
    bool isHalf;
};