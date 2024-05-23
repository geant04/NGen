#include "skybox.h"
#include "stb/stb_image.h"
#include <glad/glad.h>

const char* GetGLErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return "No error";
        case GL_INVALID_ENUM:
            return "Invalid enum";
        case GL_INVALID_VALUE:
            return "Invalid value";
        case GL_INVALID_OPERATION:
            return "Invalid operation";
        case GL_STACK_OVERFLOW:
            return "Stack overflow";
        case GL_STACK_UNDERFLOW:
            return "Stack underflow";
        case GL_OUT_OF_MEMORY:
            return "Out of memory";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "Invalid framebuffer operation";
        default:
            return "Unknown error";
    }
}

void CheckGLError(const std::string& message) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << message << ": " << GetGLErrorString(error) << " (" << error << ")" << std::endl;
    }
}

Skybox::Skybox()
{
    skyboxShader = Shader("shaders/skybox.vert.glsl", "shaders/skybox.frag.glsl");
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    //equirectangularToCubemapShader.setInt("equirectangularMap", 0);
    buildCube();
}

Skybox::~Skybox()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Skybox::buildCube()
{
    if (VAO != 0) {
        return;
    }
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    // step 1: create cube VAO
    // step 2: create skybox VAO???

    // create the VAO, VBO, and EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // bind the VAO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    // set attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void Skybox::renderCube()
{
    // render Cube
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
}

void Skybox::loadCubemap(std::vector<std::string> faces)
{
    /*
    skyboxShader = Shader("shaders/skybox.vert.glsl", "shaders/skybox.frag.glsl");
    skyboxShader.setInt("skybox", 0);
    */

    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    CheckGLError("Bug with loading in cubemap");

    int width, height, nrChannels;
    unsigned char* data;

    for (unsigned int i = 0; i < 6; i++) {
        if (!isHDR)
        {
            data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            } else {
                std::cout << "Failed to load cube map texture" << std::endl;
            }
            stbi_image_free(data);
        } else {
            std::cout << "HDR provided, skip" << std::endl;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Skybox::loadHDR(const char* texturePath)
{
    setHDR();
    
    // configure global opengl state
    // -----------------------------
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.
    
    // // set up framebuffers
    // glGenFramebuffers(1, &captureFBO);
    // glGenRenderbuffers(1, &captureRBO);

    // glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    // glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    equirectangularToCubemapShader = Shader("shaders/hdr/envCubeMap.vert.glsl", "shaders/hdr/envCubeMap.frag.glsl");

    //  load HDR
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    float *data = stbi_loadf(texturePath, &width, &height, &nrChannels, 0);

    if (data)
    {
        glGenTextures(1, &hdrTexture);
        std::cout << "hdrTexture: "  << hdrTexture << std::endl;
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL Error after glBindTexture (hdrTexture) during loading: " << std::endl;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load HDR" << std::endl;
        return;
    }
    
    // proceed to convert the HDR into a cubemap
    //loadCubemap(std::vector<std::string>());

    // glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    // glm::mat4 captureViews[] = 
    // {
    //     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    //     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    //     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    //     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    //     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    //     glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    // };

    // // render to each face of the cubemap the HDR
    // equirectangularToCubemapShader = Shader("shaders/hdr/envCubeMap.vert.glsl", "shaders/hdr/envCubeMap.frag.glsl");
    // equirectangularToCubemapShader.use();
    // equirectangularToCubemapShader.setInt("u_EquirectangularMap", 0);
    // equirectangularToCubemapShader.setMat4("projection", captureProjection);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, hdrTexture);
    // GLenum error = glGetError();
    // if (error != GL_NO_ERROR) {
    //     std::cerr << "OpenGL Error after glBindTexture (hdrTexture): " << std::endl;
    // }

    // going to be honest, not sure how this renders to each face but accept it as it is
    // glViewport(0, 0, 512, 512);
    // glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    // for (unsigned int i = 0; i < 6; i++) {
    //     equirectangularToCubemapShader.setMat4("view", captureViews[i]);
    //     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, //color attachment from the framebuffer
    //         GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
    //     std::cout << "Binding cubemap face with ID: " << cubemap << " to GL_TEXTURE_CUBE_MAP_POSITIVE_X + " << i << std::endl;
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //     renderCube();
    // }
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Skybox::draw(glm::mat4 viewMatrix, glm::mat4 projMatrix)
{
    glDepthFunc(GL_LEQUAL);
    
    glm::mat4 view = glm::mat4(glm::mat3(viewMatrix)); // remove translation -- we are always at (0, 0, 0)
    glm::mat4 projection = projMatrix;

    // equirectangularToCubemapShader.use();
    // equirectangularToCubemapShader.setMat4("view", view);
    // equirectangularToCubemapShader.setMat4("projection", projection);
    // equirectangularToCubemapShader.setInt("u_EquirectangularMap", 0);    
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, hdrTexture);
    // renderCube();

    skyboxShader.use();
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    skyboxShader.setBool("u_isHDR", isHDR);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // glBindVertexArray(VAO);
    // //glActiveTexture(GL_TEXTURE0);
    // //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    // glDrawArrays(GL_TRIANGLES, 0, 36);
    // glDepthMask(GL_TRUE);

    glDepthFunc(GL_LESS);
}