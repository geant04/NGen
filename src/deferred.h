#pragma once

#include "shader.h"

class DeferredFramebuffer
{
public:
    DeferredFramebuffer();
    ~DeferredFramebuffer();

    void Create(unsigned int WIDTH, unsigned int HEIGHT);
    void DrawLighting();
    void Clear();

    unsigned int GetGBuffer() const;
    unsigned int GetGPosition() const;
    unsigned int GetGNormal() const;
    unsigned int GetGAlbedo() const;
    unsigned int GetGMaterial() const;

    Shader GetShader() const { return deferredShader; }
    Shader &GetShaderRef() { return deferredShader; }
    Shader* GetShaderPointer() { return &deferredShader; }

private:
    unsigned int gBuffer, 
                 gPosition, 
                 gNormal, 
                 gAlbedo, 
                 gMaterial,
                 rboDepth;
    Shader deferredShader;
};