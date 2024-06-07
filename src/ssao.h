#pragma once

#include "shader.h"
#include "blur.h"
#include "camera.h"

class SSAO
{
public:
    SSAO();
    SSAO(float radius, float strength, int samples, unsigned int kernelSize);
    ~SSAO();

    void Create(unsigned int WIDTH, unsigned int HEIGHT, bool isHalf);
    void SSAOPass(const unsigned int gPosition, const unsigned int gNormal, Camera &camera);
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
private:
    Shader SSAOShader;
    BlurFramebuffer SSAOBlur;
    unsigned int SSAObuffer, SSAOfbo;
    unsigned int WIDTH, HEIGHT;
    unsigned int kernelRadius = 10;
    float SSAOradius = 0.314;
    float SSAOstrength = 1.4;
    float sss_strength = 1.0;
    int SSAOsamples = 40;
    bool doBlur = true;
    bool isHalf;
};