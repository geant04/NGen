#pragma once

#include <glad/glad.h>
#include "mesh.h"

class Cube : public Mesh
{
public:
    Cube() {};
    virtual ~Cube() {};
    void create() override;
};

void Cube::create()
{
    //  manually mapping the UVs
    this->vertices = {
        // Front face
        Vertex(glm::vec3(-1, -1,  1), glm::vec3(0, 0, 1), glm::vec2(0, 1)),
        Vertex(glm::vec3( 1, -1,  1), glm::vec3(0, 0, 1), glm::vec2(1, 1)),
        Vertex(glm::vec3( 1,  1,  1), glm::vec3(0, 0, 1), glm::vec2(1, 0)),
        Vertex(glm::vec3(-1,  1,  1), glm::vec3(0, 0, 1), glm::vec2(0, 0)),

        // Back face
        Vertex(glm::vec3(-1, -1, -1), glm::vec3(0, 0, -1), glm::vec2(0, 1)),
        Vertex(glm::vec3( 1, -1, -1), glm::vec3(0, 0, -1), glm::vec2(1, 1)),
        Vertex(glm::vec3( 1,  1, -1), glm::vec3(0, 0, -1), glm::vec2(1, 0)),
        Vertex(glm::vec3(-1,  1, -1), glm::vec3(0, 0, -1), glm::vec2(0, 0)),

        // Left face
        Vertex(glm::vec3(-1, -1, -1), glm::vec3(-1, 0, 0), glm::vec2(0, 1)),
        Vertex(glm::vec3(-1, -1,  1), glm::vec3(-1, 0, 0), glm::vec2(1, 1)),
        Vertex(glm::vec3(-1,  1,  1), glm::vec3(-1, 0, 0), glm::vec2(1, 0)),
        Vertex(glm::vec3(-1,  1, -1), glm::vec3(-1, 0, 0), glm::vec2(0, 0)),

        // Right face
        Vertex(glm::vec3( 1, -1, -1), glm::vec3(1, 0, 0), glm::vec2(0, 1)),
        Vertex(glm::vec3( 1, -1,  1), glm::vec3(1, 0, 0), glm::vec2(1, 1)),
        Vertex(glm::vec3( 1,  1,  1), glm::vec3(1, 0, 0), glm::vec2(1, 0)),
        Vertex(glm::vec3( 1,  1, -1), glm::vec3(1, 0, 0), glm::vec2(0, 0)),

        // Top face
        Vertex(glm::vec3(-1,  1, -1), glm::vec3(0, 1, 0), glm::vec2(0, 1)),
        Vertex(glm::vec3( 1,  1, -1), glm::vec3(0, 1, 0), glm::vec2(1, 1)),
        Vertex(glm::vec3( 1,  1,  1), glm::vec3(0, 1, 0), glm::vec2(1, 0)),
        Vertex(glm::vec3(-1,  1,  1), glm::vec3(0, 1, 0), glm::vec2(0, 0)),

        // Bottom face
        Vertex(glm::vec3(-1, -1, -1), glm::vec3(0, -1, 0), glm::vec2(0, 1)),
        Vertex(glm::vec3( 1, -1, -1), glm::vec3(0, -1, 0), glm::vec2(1, 1)),
        Vertex(glm::vec3( 1, -1,  1), glm::vec3(0, -1, 0), glm::vec2(1, 0)),
        Vertex(glm::vec3(-1, -1,  1), glm::vec3(0, -1, 0), glm::vec2(0, 0)),
    };

    this->indices = {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Back face
        4, 5, 6, 4, 6, 7,
        // Left face
        8, 9, 10, 8, 10, 11,
        // Right face
        12, 13, 14, 12, 14, 15,
        // Top face
        16, 17, 18, 16, 18, 19,
        // Bottom face
        20, 21, 22, 20, 22, 23
    };

    Mesh::create();
}
