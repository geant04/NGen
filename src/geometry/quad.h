#pragma once
#include <glad/glad.h>
#include "mesh.h"

class Quad : public Mesh
{
public:
    Quad() {};
    ~Quad() {};
    void create() override;
};

void Quad::create()
{
    float quadVertices[] = 
    {
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f
    };

    for (const auto &v : quadVertices)
    {
        this->rawVertices.push_back(v);
    }

    this->indices = { 0, 1, 2, 0, 2, 3 };

    // create a VBO, VAO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind and then allocate data onto GPU
    // doing sizeof(rawVertices) is incorrect, gives you 24 bytes for its 3 pointers
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, rawVertices.size() * sizeof(float), rawVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Bind 0 to positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Bind 1 to UV coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}