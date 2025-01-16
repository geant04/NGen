#pragma once

#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

class MeshRenderer
{
// note that in our deferred pipeline, we do not care about most of these
public:
    MeshRenderer();
    ~MeshRenderer();
    
    void Draw(Camera* camera);
    void Draw(
        Camera *camera, 
        unsigned int albedoID,
        unsigned int metallticID,
        unsigned int normalID,
        unsigned int roughnessID);

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
    void setParams(const glm::vec3& albedo, float metallic, float roughness);
    void setMapToggles(bool useAlbedo, bool useRoughness, bool useMetallic, bool useNormal);

    void translate(const glm::vec3& direction) 
    { 
        this->translation = direction; 
    };
    void scale(const glm::vec3& scaleAmt) 
    { 
        this->scaleAmt = scaleAmt; 
    };
    void rotate(float angle, const glm::vec3& axis)
    {
        this->rotationAxis = axis;
        this->rotateTheta = angle;
    }

    unsigned int getAlbedoID() const { return albedoMap.getTextureID(); };
    unsigned int getMetallicID() const { return metallicMap.getTextureID(); };
    unsigned int getNormalID() const { return normalMap.getTextureID(); };
    unsigned int getRoughnessID() const { return roughnessMap.getTextureID(); };
    
    bool useAlbedoMap = false;
    bool useRoughnessMap = false;
    bool useMetallicMap = false;
    bool useNormalMap = false;
    bool showModel = true;
    bool showEnv = true;

private:
    Shader shader;
    Mesh *mesh;

    Texture2D albedoMap;
    Texture2D normalMap;
    Texture2D metallicMap;
    Texture2D roughnessMap;

    // default PBR values, excluding maps
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ambientOcclusion;

    // transformations
    glm::mat4 model;
    glm::vec3 translation;
    glm::vec3 scaleAmt;
    glm::vec3 rotationAxis;
    float rotateTheta;
};