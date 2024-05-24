#include "mesh.h"
#include <glad/glad.h>
#include "tiny_obj_loader.h"
#include <iostream>

void Mesh::create()
{
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);

    // full process:
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // bind indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    // vertex texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    std::cout << "Mesh created" << std::endl;
    std::cout << "Vertices: " << vertices.size() << std::endl;
    std::cout << "Indices: " << indices.size() << std::endl;

    glBindVertexArray(0);
}

Mesh::Mesh()
    : vertices(std::vector<Vertex>()), 
      indices(std::vector<unsigned int>()), textures(std::vector<Texture>()),
      hasCubeMap(false),
      hasIrradiance(false)
{
    // normally i'd have create here but let's wait for a moment
    // this is for the obj loader
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    create();
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// obj loading
void Mesh::LoadObj(const char* path)
{
    // load obj file, for now we don't have to worry about materials
    tinyobj::attrib_t attrib;  
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string base_dir = "";
    std::string mtl_dir = "";

    std::string err;
    std::string warn;

    bool ret = tinyobj::LoadObj(
        &attrib, 
        &shapes, 
        &materials, 
        &warn,
        &err, 
        path);

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "ERR: " << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Failed to load/parse .obj file" << std::endl;
        return;
    }

    for (int i = 0; i < shapes.size(); i ++) {

        tinyobj::shape_t &shape = shapes[i];
        tinyobj::mesh_t &mesh = shape.mesh;

        for (int j = 0; j < mesh.indices.size(); j++) {
            tinyobj::index_t index = mesh.indices[j];
            Vertex vertex;


            vertex.Position = glm::vec3(
                attrib.vertices[index.vertex_index * 3 + 0],
                attrib.vertices[index.vertex_index * 3 + 1],
                attrib.vertices[index.vertex_index * 3 + 2]
            );

            if (!attrib.normals.empty()) {
                vertex.Normal = glm::vec3(
                    attrib.normals[index.normal_index * 3 + 0],
                    attrib.normals[index.normal_index * 3 + 1],
                    attrib.normals[index.normal_index * 3 + 2]
                );
            } else {
                vertex.Normal = glm::vec3(0.0f);
            }

            if (!attrib.texcoords.empty()) {
                vertex.TexCoords = glm::vec2(
                    attrib.texcoords[index.texcoord_index * 2],
                    attrib.texcoords[index.texcoord_index * 2 + 1]
                );
            } else {
                vertex.TexCoords = glm::vec2(0.0f);
            }

            vertices.push_back(vertex);
            indices.push_back(static_cast<unsigned int>(indices.size()));
        }
    }
}

void Mesh::bindCubeMap(unsigned int cubemapID)
{
    this->cubemapID = cubemapID;
    this->hasCubeMap = true;
}

void Mesh::bindIrradianceMap(unsigned int irradianceID)
{
    this->irradianceID = irradianceID;
    this->hasIrradiance = true;
}

// draw function
void Mesh::Draw()
{
    glBindVertexArray(VAO);
    if (hasCubeMap) {
        std::cout << "cubemap detected, bind" << std::endl;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
    }
    if (hasIrradiance)  {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceID);
    }
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}