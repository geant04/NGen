#include "mesh_renderer.h"

MeshRenderer::MeshRenderer()
    : translation(glm::vec3(0)), scaleAmt(glm::vec3(1)), rotationAxis(glm::vec3(0, 1, 0)),
    rotateTheta(0), albedo(glm::vec3(0.5, 0, 0)), metallic(0.5), roughness(0.5), ambientOcclusion(1.0)
{}

MeshRenderer::~MeshRenderer()
{}

void MeshRenderer::Draw(
    Camera *camera, 
    unsigned int albedoID,
    unsigned int metallticID,
    unsigned int normalID,
    unsigned int roughnessID)
{
    shader.use();
    glm::mat4 model = glm::mat4(1.0);

    model = glm::translate(model, translation);
    model = glm::scale(model, scaleAmt);
    model = glm::rotate(model, rotateTheta, rotationAxis);

    shader.setMat4("model", model);
    shader.setMat4("view", camera->getViewMatrix());
    shader.setMat4("projection", camera->getProjectionMatrix());

    // you'd need this albedo data in here, otherwise the forward pass won't work
    shader.setVec3("u_albedo", albedo);
    shader.setFloat("u_roughness", roughness);
    shader.setFloat("u_metallic", metallic);

    // useProps
    shader.setBool("useAlbedoMap", useAlbedoMap);
    shader.setBool("useRoughnessMap", useRoughnessMap);
    shader.setBool("useMetallicMap", useMetallicMap);
    shader.setBool("useNormalMap", useNormalMap);

    // these maps are necessary for loading in proper PBR material data
    if (albedoID != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedoID);
        shader.setInt("albedoMap", 0);
    }

    if (metallticID != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metallticID);
        shader.setInt("metallicMap", 1);
    }

    if (normalID != -1) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normalID);
        shader.setInt("normalMap", 2);
    }

    if (roughnessID != -1)  {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughnessID);
        shader.setInt("roughnessMap", 3);
    }

    mesh->Draw();
}

void MeshRenderer::Draw(Camera *camera)
{  
    Draw(camera, 
        albedoMap.getTextureID(),
        metallicMap.getTextureID(),
        normalMap.getTextureID(),
        roughnessMap.getTextureID());
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

void MeshRenderer::setMapToggles(bool useAlbedo, bool useRoughness, bool useMetallic, bool useNormal)
{
    this->useAlbedoMap = useAlbedo;
    this->useRoughnessMap = useRoughness;
    this->useMetallicMap = useMetallic;
    this->useNormalMap = useNormal;
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
