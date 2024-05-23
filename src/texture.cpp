#include "texture.h"
#include <iostream>

Texture2D::Texture2D()
{
    glGenTextures(1, &textureID);
}

Texture2D::Texture2D(const char* texturePath)
{
    loadTexture(texturePath);
}

void Texture2D::loadTexture(const char* texturePath)
{
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format;
        GLenum internalFormat;
        // check number of channels, change format
        if (nrChannels == 1) {
            format = GL_RED;
            internalFormat = GL_R8;
        } else if (nrChannels == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB8;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        }

        // bind texture, set parameters
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL Error after trying to load in textureID: " << std::endl;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    loaded = true;
}

