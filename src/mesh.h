#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
};

// should include camera transformation matrix stuff
// these need to be passed into the shader, but we can
// worry abt that later

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh();
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    ~Mesh();
    void Draw();
    void LoadObj(const char* path);
    void create();
    void bindCubeMap(unsigned int cubemapID);
    void bindIrradianceMap(unsigned int irradianceID);
    void bindSpecularMap(unsigned int specularID);
    void bindBrdfLUT(unsigned int brdfLUTID);
private:
    unsigned int VAO, VBO, EBO, 
        cubemapID, 
        irradianceID,
        specularID,
        brdfLUTID;
    bool hasCubeMap;
    bool hasIrradiance;
    bool hasSpecular;
    bool hasBRDFLUT;
};