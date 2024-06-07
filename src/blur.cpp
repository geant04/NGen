#include "blur.h"

#define M_PI           3.14159265358979323846

std::vector<GLfloat> computeGaussianKernel(int radius) {
    float sigma = glm::max(radius * 0.5f, 1.f);
    float sigma2 = sigma * sigma;
    int kernelWidth = radius * 2 + 1;
    std::vector<GLfloat> kernel =
            std::vector<GLfloat>(kernelWidth, 0.f);

    float sum = 0;

    for (int x = -radius; x < radius; x++) {
        float sign = 1.0 / sqrt(2 * M_PI * sigma2);
        float param = -1.0 * (x * x) / (2.0 * sigma2);
        float gauss = sign * exp(param);

        kernel[x + radius] = gauss;
        sum += gauss;
    }

    // new weights....
    std::vector<GLfloat> newKernel =
        std::vector<GLfloat>(kernelWidth / 2 + 1, 0.f);
    int newKernelWidth = kernelWidth / 2 + 1;

    for (int i = 0; i < newKernelWidth; i += 2) {
        float weight1 = kernel[i];
        float weight2 = kernel[i + 1];

        float weightL = weight1 + weight2;

        newKernel[i / 2] = weightL;
    }

    for (int i = 0; i < (newKernelWidth); i++) {
        newKernel[i] /= sum;
    }

    return newKernel;
}

BlurFramebuffer::BlurFramebuffer()
    : blurFbo(0), blurBuffer(0)
{
    BlurShader = Shader("shaders/ssao/ssao.vert.glsl", "shaders/deferred/blur.frag.glsl");
}

BlurFramebuffer::BlurFramebuffer(int kernelSize)
 : kernelRadius(kernelSize)
{
    BlurShader = Shader("shaders/ssao/ssao.vert.glsl", "shaders/deferred/blur.frag.glsl");
    BlurShader.use();
    BlurShader.setInt("sampleTexture", 0);
    BlurShader.setInt("kernel", 1);
    BlurShader.setInt("kernelRadius", kernelRadius);

    GenerateKernel();
}

BlurFramebuffer::~BlurFramebuffer()
{
    glDeleteFramebuffers(1, &blurFbo);
    glDeleteTextures(1, &blurBuffer);
}

void BlurFramebuffer::AssignTarget(unsigned int target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurFramebuffer::GenerateKernel()
{
    kernel = computeGaussianKernel(kernelRadius);
    gaussKernel.init();
    gaussKernel.bufferPixelData(kernel.size(), 1, kernel.data(), 1);
}

void BlurFramebuffer::Create(unsigned int WIDTH, unsigned int HEIGHT, bool isHalf)
{
    this->isHalf = isHalf;
    this->WIDTH = isHalf ? WIDTH / 2 : WIDTH;
    this->HEIGHT = isHalf ? HEIGHT / 2 : HEIGHT;

    glGenFramebuffers(1, &blurFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);

    glGenTextures(1, &blurBuffer);
    glBindTexture(GL_TEXTURE_2D, blurBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, this->WIDTH, this->HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurBuffer, 0);

    // GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
    // glDrawBuffers(1, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    std::cout << "blur setup complete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlurFramebuffer::BlurPass(unsigned int targetFBO, unsigned int targetTextureID)
{
    if (isHalf)
    {
        glViewport(0, 0, WIDTH, HEIGHT);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, targetTextureID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gaussKernel.getTextureID());
    
    BlurShader.use();
    BlurShader.setBool("isAO", isAO);
    BlurShader.setInt("u_PingPong", 0);
    Skybox::renderQuad();

    // PASS 2
    // rewrite to SSAO fbo
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // re-assign sample texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurBuffer);

    BlurShader.use();
    BlurShader.setBool("isAO", isAO);
    BlurShader.setInt("u_PingPong", 1);
    Skybox::renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (isHalf)
    {
        glViewport(0, 0, WIDTH * 2, HEIGHT * 2);
    }
}