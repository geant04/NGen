#pragma once
#include <string>
#include <vector>
#include "Shader.h"

class Skybox
{
public:
    Skybox();
    ~Skybox();
    void draw(glm::mat4 viewMatrix, glm::mat4 projMatrix);
    void loadCubemap(std::vector<std::string> faces);
    unsigned int getCubemap() {
        return cubemap;
    }
private:
    unsigned int VAO, VBO, EBO;
    unsigned int cubemap = 0;
    Shader skyboxShader;
};
