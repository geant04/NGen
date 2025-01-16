#pragma once

#include "shader.h"

class FrameBuffer
{
public:
    FrameBuffer() {};
    ~FrameBuffer();

    unsigned int getID() const { return objectID; };
    unsigned int getBufferID() const { return bufferID; };
    void Create(unsigned int WIDTH, unsigned int HEIGHT, bool isHalf);
private:
    unsigned int objectID;
    unsigned int bufferID;
    unsigned int WIDTH, HEIGHT;
    bool isHalf;
};

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &objectID);
    glDeleteTextures(1, &bufferID);
}

void FrameBuffer::Create(unsigned int WIDTH, unsigned int HEIGHT, bool isHalf)
{
    this->isHalf = isHalf;
    this->WIDTH = isHalf ? WIDTH / 2 : WIDTH;
    this->HEIGHT = isHalf ? HEIGHT / 2 : HEIGHT;

    // generate buffer IDs
    glGenFramebuffers(1, &objectID);
    glBindFramebuffer(GL_FRAMEBUFFER, objectID);

    // build cool awesome buffer values
    glGenTextures(1, &bufferID);
    glBindTexture(GL_TEXTURE_2D, bufferID);

    // initialize our buffer
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, this->WIDTH, this->HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferID, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    
    std::cout << "default framebuffer setup complete" << std::endl;

    // bind to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
