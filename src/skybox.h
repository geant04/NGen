#pragma once
#include <string>
#include <vector>
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Skybox
{
public:
    Skybox();
    ~Skybox();
    void draw(glm::mat4 viewMatrix, glm::mat4 projMatrix);
    void loadCubemap(std::vector<std::string> faces);
    void loadHDR(const char* path);
    void buildCube();
    void renderCube();
    unsigned int getCubemap() {
        return cubemap;
    }
    unsigned int getHDRTexture() {
        return hdrTexture;
    }
    void setHDR() {
        this->isHDR = true;
    }

private:
    unsigned int VAO, VBO, EBO;
    unsigned int cubemap;
    unsigned int hdrTexture;
    unsigned int captureFBO, captureRBO;
    bool isHDR;
    Shader skyboxShader;
    Shader equirectangularToCubemapShader;
};
