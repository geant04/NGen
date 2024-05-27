#include "mesh_renderer.h"

MeshRenderer::MeshRenderer()
{
    translation = glm::vec3(0.);
    scaleAmt = glm::vec3(1.);
}

MeshRenderer::~MeshRenderer()
{}

void MeshRenderer::Draw(Camera *camera)
{  
    shader.use();
    // TO DO: store the model matrix in the mesh
    glm::mat4 model = glm::mat4(1.0);

    model = glm::translate(model, translation);
    model = glm::scale(model, scaleAmt);

    shader.setMat4("model", model);
    shader.setMat4("view", camera->getViewMatrix());
    shader.setMat4("projection", camera->getProjectionMatrix());

    // you'd need this albedo data in here, otherwise the forward pass won't work
    shader.setVec3("u_albedo", albedo);
    shader.setFloat("u_roughness", roughness);
    shader.setFloat("u_metallic", metallic);

    // leave this here for now
    shader.setBool("useAlbedoMap", true);

    // these maps are necessary for loading in proper PBR material data
    if (albedoMap.getTextureID() != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoMap.getTextureID());
        shader.setInt("albedoMap", 0);
    }

    if (metallicMap.getTextureID() != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metallicMap.getTextureID());
        shader.setInt("metallicMap", 1);
    }

    if (normalMap.getTextureID() != -1) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normalMap.getTextureID());
        shader.setInt("normalMap", 2);
    }

    if (roughnessMap.getTextureID() != -1)  {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessMap.getTextureID());
        shader.setInt("roughnessMap", 3);
    }

    mesh->Draw();
}

void MeshRenderer::LoadMaterials(const char* albedoPath,
                                 const char* normalPath,
                                 const char* metallicPath,
                                 const char* roughnessPath)
{
    albedoMap.loadTexture(albedoPath);
    normalMap.loadTexture(normalPath);
    metallicMap.loadTexture(metallicPath);
    roughnessMap.loadTexture(roughnessPath);
}

void MeshRenderer::SetMesh(Mesh *m)
{
    this->mesh = m;
}

void MeshRenderer::setParams(glm::vec3 albedo, float metallic, float roughness)
{
    this->albedo = albedo;
    this->metallic = metallic;
    this->roughness = roughness;
}

void MeshRenderer::LoadShader(const char* vert, const char* frag)
{
    this->shader = Shader(vert, frag);
    std::cout << "loaded shader" << std::endl;
}

void MeshRenderer::LoadAlbedo(const char* albedoPath)
{
    albedoMap.loadTexture(albedoPath);
}
