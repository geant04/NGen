#include "ssao.h"

SSAO::SSAO()
{
    SSAOShader = Shader("shaders/ssao/ssao.vert.glsl", "shaders/ssao/ssao.frag.glsl");
}

// SSAO::SSAO(SSAO::Settings settings)
//     : settings(settings)
// {
//     SSAOShader = Shader("shaders/ssao/ssao.vert.glsl", "shaders/ssao/ssao.frag.glsl");
// }

SSAO::~SSAO()
{
    glDeleteFramebuffers(1, &SSAOfbo);
    glDeleteTextures(1, &SSAObuffer);
}

void SSAO::Create(const unsigned int WIDTH, const unsigned int HEIGHT, const bool isHalf)
{
    this->isHalf = isHalf;
    this->WIDTH = isHalf ? WIDTH / 2 : WIDTH;
    this->HEIGHT = isHalf ? HEIGHT / 2 : HEIGHT;

    glGenFramebuffers(1, &SSAOfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, SSAOfbo);

    glGenTextures(1, &SSAObuffer);
    glBindTexture(GL_TEXTURE_2D, SSAObuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, this->WIDTH, this->HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAObuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    SSAOBlur = BlurFramebuffer(kernelRadius);
    SSAOBlur.Create(WIDTH, HEIGHT, true);
    SSAOBlur.isAOEnabled(true);
}

void SSAO::SSAOPass(const unsigned int gPosition, const unsigned int gNormal, Camera &camera, Mesh &quad)
{
    if (isHalf)
    {
        glViewport(0, 0, WIDTH, HEIGHT);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, SSAOfbo);

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);

    // use SSAO shader
    SSAOShader.use();
    SSAOShader.setInt("gPosition", 0);
    SSAOShader.setInt("gNormal", 1);
    SSAOShader.setMat4("projection", camera.getProjectionMatrix());
    SSAOShader.setMat4("view", camera.getViewMatrix());
    SSAOShader.setInt("samples", SSAOsamples);
    SSAOShader.setFloat("radius", SSAOradius);
    SSAOShader.setFloat("aoStrength", SSAOstrength);
    SSAOShader.setFloat("sssStrength", SSAOinvStrength);
    quad.Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (doBlur && false)
    {
        //SSAOBlur.AssignTarget(SSAObuffer);
        SSAOBlur.BlurPass(SSAOfbo, SSAObuffer, quad);
    }

    if (isHalf)
    {
        glViewport(0, 0, WIDTH * 2, HEIGHT * 2);
    }
}