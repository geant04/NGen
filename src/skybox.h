#pragma once
#include <string>
#include <vector>
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"

class Skybox
{
public:
    Skybox();
    ~Skybox();
    void draw(Camera *camera);
    void loadCubemap(std::vector<std::string> faces);
    void loadHDR(const char* path);
    void createIrradianceMap();
    void createSpecularMap();
    void createBRDFLUT();
    void buildCube();
    void renderCube();
    void renderQuad();
    void initCaptures();
    unsigned int getCubemap() {
        return cubemap;
    }
    unsigned int getHDRTexture() {
        return hdrTexture;
    }
    unsigned int getIrradianceMap() {
        return irradianceMap;
    }
    unsigned int getSpecularMap() {
        return specularMap;
    }
    unsigned int getBRDFLUT() {
        return brdfTexture;
    }
private:
    unsigned int VAO, VBO;
    unsigned int cubemap;
    unsigned int irradianceMap;
    unsigned int specularMap;
    unsigned int brdfTexture;
    unsigned int hdrTexture;
    unsigned int captureFBO, captureRBO;
    unsigned int quadVAO, quadVBO;
    bool isHDR;
    Shader skyboxShader;
    Shader equirectangularToCubemapShader;
    Shader irradianceShader;
    Shader specularShader;
    Shader brdfLUTShader;
    glm::mat4 captureProjection;
};
