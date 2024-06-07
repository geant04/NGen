#pragma once

#include "shader.h"

class DeferredFramebuffer
{
public:
    DeferredFramebuffer();
    ~DeferredFramebuffer();

    void Create(unsigned int WIDTH, unsigned int HEIGHT);
    void Draw();
    void Clear();

// TO DO: MERGE AO/THICKNESS DETAILS INTO gMaterial or distribute them to gAlbedo and gNormal

    unsigned int GetGBuffer() const { return gBuffer; }
    unsigned int GetGPosition() const { return gPosition; }
    unsigned int GetGNormal() const { return gNormal; }
    unsigned int GetGAlbedo() const { return gAlbedo; }
    unsigned int GetGMaterial() const { return gMaterial; }

    Shader GetShader() const { return deferredShader; }
    Shader &GetShader() { return deferredShader; }

private:
    unsigned int gBuffer, 
                 gPosition, 
                 gNormal, 
                 gAlbedo, 
                 gMaterial,
                 rboDepth;
    Shader deferredShader;
};