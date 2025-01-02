#pragma once

#include "shader.h"
#include "framebuffer.h"
#include "camera.h"
#include "mesh.h"

class SSR
{
public:
    SSR() {};
    SSR(unsigned int WIDTH, unsigned int HEIGHT) : thickness(0.0075), maxDistance(10.0)
    {
        SSRShader = Shader("shaders/ssao/ssao.vert.glsl", "shaders/ssr/ssr.frag.glsl");
        SSRfbo = FrameBuffer();
        SSRfbo.Create(WIDTH, HEIGHT, false);
    };
    ~SSR() {}; // should automatically destroy SSRfbo information

    void SSRPass(
        const unsigned int gPosition, 
        const unsigned int gNormal, 
        const unsigned int gAlbedo,
        const unsigned int gMaterial,
        const Camera &camera, 
        Mesh &mesh);

    FrameBuffer* GetFBO() { return &SSRfbo; };
    float maxDistance;
    float thickness;

private:
    Shader SSRShader;
    FrameBuffer SSRfbo;
};

void SSR::SSRPass(
    const unsigned int gPosition, 
    const unsigned int gNormal, 
    const unsigned int gAlbedo,
    const unsigned int gMaterial,
    const Camera &camera, 
    Mesh &mesh)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // SSRfbo.getID()

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gMaterial);
    
    glm::mat4 viewMat = camera.getViewMatrix();
    // use SSR shader
    SSRShader.use();
    SSRShader.setMat4("projection", camera.getProjectionMatrix());
    SSRShader.setMat4("view", viewMat);
    SSRShader.setVec3("cameraPos", camera.eye);
    SSRShader.setInt("gPosition", 0);
    SSRShader.setInt("gNormal", 1);
    SSRShader.setInt("gAlbedo", 2);
    SSRShader.setInt("gMaterial", 3);
    SSRShader.setFloat("maxDistance", maxDistance);
    SSRShader.setFloat("thickness", thickness);
    SSRShader.setFloat("near", camera.near);
    SSRShader.setFloat("far", camera.far);
    
    // render, ideally should be a quad that's passed in
    mesh.Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}