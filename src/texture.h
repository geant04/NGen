#pragma once

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <string>

class Texture2D
{
public:
    Texture2D();
    Texture2D(const char* texturePath);

    void loadTexture(const char* texturePath);
    bool loaded = false;

    unsigned int getTextureID() {
        return textureID;
    }
private:
    unsigned int textureID;
};