#pragma once

#include "shader.h"
#include "skybox.h"
#include "texture.h"

class BlurFramebuffer
{
public:
    BlurFramebuffer();
    BlurFramebuffer(int kernelSize);
    ~BlurFramebuffer();

    void Create(unsigned int WIDTH, unsigned int HEIGHT, bool isHalf);
    void BlurPass(unsigned int targetFBO, unsigned int targetTexture);
    void AssignTarget(unsigned int target);
    void CreateShader(const char* vertexPath, const char* fragmentPath);
    void isAOEnabled(bool isAO) { this->isAO = isAO; }
    void GenerateKernel();

private:
    Shader BlurShader;
    unsigned int blurFbo, blurBuffer;
    unsigned int WIDTH, HEIGHT;

    unsigned int kernelRadius;
    std::vector<GLfloat> kernel = {};
    Texture2D gaussKernel;

    bool isHalf;
    bool isAO;
};