#pragma once

#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

class MeshRenderer
{
private:
    Shader shader;
    Mesh *mesh;
    Texture2D albedoMap;
    Texture2D normalMap;
    Texture2D metallicMap;
    Texture2D roughnessMap;
    glm::vec3 albedo;
    float metallic;
    float roughness;
    glm::mat4 model;
    glm::vec3 translation;
    glm::vec3 scaleAmt;
    glm::vec3 rotationAxis;
    float rotateTheta;

// note that in our deferred pipeline, we do not care about most of these
public:
    MeshRenderer();
    ~MeshRenderer();
    void Draw(Camera* camera);
    void LoadShader(const char* vertPath, const char* fragPath);
    void SetMesh(Mesh *m);
    void LoadMaterials(
        const char* albedoPath, 
        const char* normalMap,
        const char* metallicPath, 
        const char* roughnessPath);
    void LoadAlbedo(const char* albedoPath);
    void LoadNormal(const char* albedoPath);
    void LoadMetalMap(const char* albedoPath);
    void LoadRoughMap(const char* albedoPath);
    void setParams(glm::vec3 albedo, float metallic, float roughness);
    void translate(glm::vec3 direction)
    {
        this->translation = direction;
        std::cout << "set translate" << std::endl;
    };
    void scale(glm::vec3 scaleAmt)
    {
        this->scaleAmt = scaleAmt;
    }
    void rotate(float angle, glm::vec3 axis)
    {
        this->rotationAxis = axis;
        this->rotateTheta = angle;
    }
};