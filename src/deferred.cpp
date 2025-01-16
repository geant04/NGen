#include "deferred.h"
#include <iostream>

DeferredFramebuffer::DeferredFramebuffer()
: gBuffer(-1), gPosition(-1), gNormal(-1), gAlbedo(-1), gMaterial(-1), rboDepth(-1)
{
    deferredShader = Shader("shaders/deferred/lighting.vert.glsl", "shaders/deferred/lighting.frag.glsl");
    deferredShader.use();
    deferredShader.setInt("gPosition", 0);
    deferredShader.setInt("gNormal", 1);
    deferredShader.setInt("gAlbedo", 2);
    deferredShader.setInt("gMaterial", 3);
    deferredShader.setInt("u_IrradianceMap", 4);
    deferredShader.setInt("u_SpecularMap", 5);
    deferredShader.setInt("u_BRDFLUT", 6);
    deferredShader.setInt("u_SSAO", 7);
}

DeferredFramebuffer::~DeferredFramebuffer()
{
    glDeleteFramebuffers(1, &gBuffer);
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gAlbedo);
    glDeleteTextures(1, &gMaterial);
    glDeleteRenderbuffers(1, &rboDepth);
}

void DeferredFramebuffer::Create(unsigned int WIDTH, unsigned int HEIGHT)
{
    glEnable(GL_DEPTH_TEST);

    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // generate position buffer -- id: 0
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    std::cout << "gPosition: " << gPosition << std::endl;

    // generate normal buffer -- id: 1
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    std::cout << "gNormal: " << gNormal << std::endl;

    // generate albedo buffer -- id: 2
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
    std::cout << "gAlbedo: " << gAlbedo << std::endl;

    // generate material buffer -- id: 3
    glGenTextures(1, &gMaterial);
    glBindTexture(GL_TEXTURE_2D, gMaterial);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMaterial, 0);
    std::cout << "gMaterial: " << gMaterial << std::endl;

    // write to four textures, but I'm going to be honest we probably don't need the fourth one
    GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredFramebuffer::DrawLighting()
{
    
}

unsigned int DeferredFramebuffer::GetGBuffer() const { return gBuffer; }
unsigned int DeferredFramebuffer::GetGPosition() const { return gPosition; }
unsigned int DeferredFramebuffer::GetGNormal() const { return gNormal; }
unsigned int DeferredFramebuffer::GetGAlbedo() const { return gAlbedo; }
unsigned int DeferredFramebuffer::GetGMaterial() const { return gMaterial; }
unsigned int DeferredFramebuffer::GetDepth() const { return rboDepth; }