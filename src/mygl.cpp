#include "mygl.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "mesh.h"
#include "skybox.h"
#include "texture.h"
#include "mesh_renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

# define M_PI           3.14159265358979323846

MyGL::MyGL()
{
    init();
}

MyGL::~MyGL()
{
    // clean up
}

// buffer set ups for deferred rendering
unsigned int gBuffer, gPosition, gNormal, gAlbedo, gMaterial;
unsigned int gDepthBuffer;
unsigned int SSAObuffer, SSAOfbo;
unsigned int blurbuffer, blurfbo;

// camera
const int WIDTH = 1280;
const int HEIGHT = 720;
Camera camera(WIDTH, HEIGHT);
glm::vec2 lastMousePos = glm::vec2(WIDTH / 2, HEIGHT / 2);
bool firstMouse = true;
bool mousePressed = false;
bool wantCapture = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// display settings
bool turntable = false;
bool angledTurn = false;
bool showSSAO = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    // update the camera's width and height
    camera.setWidth(width);
    camera.setHeight(height);
}

void processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    float cameraSpeed = 5.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.translateForward(cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.translateForward(-cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.translateRight(-cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.translateRight(cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.translateUp(-cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.translateUp(cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.rotateRight(-cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.rotateRight(cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.rotateUp(-cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.rotateUp(cameraSpeed);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    glm::vec2 pos = glm::vec2(xpos, ypos);

    if (firstMouse)
    {
        lastMousePos = pos;
        firstMouse = false;
    }

    glm::vec2 offset = pos - lastMousePos;
    lastMousePos = pos;

    float sensitivity = 0.2f;
    offset *= sensitivity;

    if (mousePressed && !wantCapture)
    {
        camera.rotatePhi(-offset.x);
        camera.rotateTheta(-offset.y);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        //lastMousePos = glm::vec2(xpos, ypos);
        mousePressed = true;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        mousePressed = false;
    }
}

void setupGBuffer();

void setupSSAO();

std::vector<GLfloat> computeGaussianKernel(int radius) {
    float sigma = glm::max(radius * 0.5f, 1.f);
    float sigma2 = sigma * sigma;
    int kernelWidth = radius * 2 + 1;
    std::vector<GLfloat> kernel =
            std::vector<GLfloat>(kernelWidth, 0.f);

    float sum = 0;

    for (int x = -radius; x < radius; x++) {
        float sign = 1.0 / sqrt(2 * M_PI * sigma2);
        float param = -1.0 * (x * x) / (2.0 * sigma2);
        float gauss = sign * exp(param);

        kernel[x + radius] = gauss;
        sum += gauss;
    }

    // new weights....
    std::vector<GLfloat> newKernel =
        std::vector<GLfloat>(kernelWidth / 2 + 1, 0.f);
    int newKernelWidth = kernelWidth / 2 + 1;

    for (int i = 0; i < newKernelWidth; i += 2) {
        float weight1 = kernel[i];
        float weight2 = kernel[i + 1];

        float weightL = weight1 + weight2;

        newKernel[i / 2] = weightL;
    }

    for (int i = 0; i < (newKernelWidth); i++) {
        newKernel[i] /= sum;
    }

    return newKernel;
}

void setupBlur();

void MyGL::init() 
{
    // glfw initialization
    // --------------------------

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // window creation
    // --------------------------

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "NGEN 0.0", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    // register callback -- handle window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // register callback -- handle mouse movement
    glfwSetCursorPosCallback(window, mouse_callback);
    // register callback -- handle mouse button press
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // load function pointers??
    // --------------------------

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Model loading
    // --------------------------

    Mesh ourMesh;
    std::string modelName = "wahoo";
    std::string modelPath = "models/" + modelName + ".obj";
    ourMesh.LoadObj(modelPath.c_str());
    ourMesh.create();

    MeshRenderer meshRenderer;
    meshRenderer.SetMesh(&ourMesh);
    meshRenderer.LoadMaterials("textures/jade/jade_albedo.jpg",
                               "textures/pbrWood/mahogfloor_normal.png",
                               "textures/pbrGold/gold-scuffed_metallic.png",
                               "textures/pbrGold/gold-scuffed_roughness.png");
    meshRenderer.LoadShader("shaders/deferred/gbuffer.vert.glsl", "shaders/deferred/gbuffer.frag.glsl");

    Mesh groundMesh;
    groundMesh.LoadObj("models/cube.obj");
    groundMesh.create();
    MeshRenderer groundRenderer;
    groundRenderer.SetMesh(&groundMesh);
    groundRenderer.LoadMaterials("textures/pbrCopper/Copper-scuffed_basecolor-boosted.png",
                               "textures/pbrCopper/Copper-scuffed_normal.png",
                               "textures/pbrCopper/Copper-scuffed_metallic.png",
                               "textures/pbrCopper/Copper-scuffed_roughness.png");
    groundRenderer.LoadShader("shaders/deferred/gbuffer.vert.glsl", "shaders/deferred/gbuffer.frag.glsl");
    groundRenderer.translate(glm::vec3(0, -4.75, 0));
    groundRenderer.scale(glm::vec3(8, 0.75, 8));
    

    Skybox envMap;
    envMap.loadHDR("textures/hdr/hangar_interior_4k.hdr");
    envMap.createIrradianceMap();
    envMap.createSpecularMap();
    envMap.createBRDFLUT();

    // Viewport resizing necessary after a bunch of the crap i did
    // --------------------------

    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    // iniitalize imgui
    // --------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // constants for IMGUI
    // --------------------------
    glm::vec3 u_Albedo = glm::vec3(0.5f, 0.0f, 0.0f);
    float u_Roughness = 0.5f;
    float u_Metallic = 0.5f;
    float u_AmbientOcclusion = 1.0f;
    float color[3] = {u_Albedo.r, u_Albedo.g, u_Albedo.b};
    float background[3] = {0.0f, 0.0f, 0.0f};

    bool useAlbedoMap = false;
    bool useNormalMap = false;
    bool useMetallicMap = false;
    bool useRoughnessMap = false;

    bool showModel = true;
    bool showEnv = true;
    
    bool enableSSAO = true;
    bool showSSAODebug = true;
    float SSAOradius = 0.314;
    float SSAOstrength = 1.4;
    int SSAOsamples = 40;

    glm::vec3 SSSColor = glm::vec3(1.f, 1.0f, 1.0f);
    float sssColor[3] = {SSSColor.r, SSSColor.g, SSSColor.b};
    float sss_distortion = 0.352;
    float sss_scale = 1.0;
    float sss_ambient = 0.2;
    float sss_glow = 2;
    float sss_strength = 1.0;

    // render loop
    // --------------------------

    // TO DO: ABSTRACT SSAO, DEFERRED RENDERING, AND GAUSSIAN BLUR

    // deferred rendering setup
    Shader deferredShader;
    deferredShader = Shader("shaders/deferred/deferred.vert.glsl", "shaders/deferred/deferred.frag.glsl");
    deferredShader.use();
    deferredShader.setInt("gPosition", 0);
    deferredShader.setInt("gNormal", 1);
    deferredShader.setInt("gAlbedo", 2);
    deferredShader.setInt("gMaterial", 3);
    deferredShader.setInt("u_IrradianceMap", 4);
    deferredShader.setInt("u_SpecularMap", 5);
    deferredShader.setInt("u_BRDFLUT", 6);
    deferredShader.setInt("u_SSAO", 7);

    // GBuffer setup
    glEnable(GL_DEPTH_TEST);
    setupGBuffer();

    // SSAO setup
    // the vert shaders for SSAO and brdfLUT are identical

    Shader SSAOShader;
    SSAOShader = Shader("shaders/ssao/ssao.vert.glsl", "shaders/ssao/ssao.frag.glsl");
    SSAOShader.use();
    SSAOShader.setInt("gPosition", 0);
    SSAOShader.setInt("gNormal", 1);
    setupSSAO();

    // Blur setup
    Shader BlurShader;
    BlurShader = Shader("shaders/ssao/ssao.vert.glsl", "shaders/deferred/blur.frag.glsl");
    BlurShader.use();
    BlurShader.setInt("sampleTexture", 0);
    BlurShader.setInt("kernel", 1);
    BlurShader.setInt("kernelRadius", 8);

    std::vector<GLfloat> kernel = computeGaussianKernel(8);
    Texture2D gaussKernel;
    gaussKernel.init();
    gaussKernel.bufferPixelData(9, 1, kernel.data(), 1);
    setupBlur();

    // for testing purposes
    Shader testQuad;
    testQuad = Shader("shaders/ssao/ssao.vert.glsl", "shaders/ssao/testSAO.glsl");
    testQuad.use();
    testQuad.setInt("testTXT", 0);

    glm::vec3 testPositions[9] = {
        glm::vec3( 0., 0., 0.),
        glm::vec3(-1, 0., 0.),
        glm::vec3( 1., 0., 0.),
        glm::vec3(-1., 0., 1.),
        glm::vec3( 0., 0., 1.),
        glm::vec3( 1., 0., 1.),
        glm::vec3(-1., 0., -1.),
        glm::vec3( 0., 0., -1.),
        glm::vec3( 1., 0., -1.)
    };

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, camera, deltaTime);

        // frame logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        wantCapture = io.WantCaptureMouse;

        // draw scene to G-buffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (turntable)
        {
            meshRenderer.rotate(currentFrame, glm::vec3(0, 1, 0));
        }

        meshRenderer.setParams(u_Albedo, u_Metallic, u_Roughness);
        meshRenderer.setMapToggles(useAlbedoMap, useRoughnessMap, useMetallicMap, useNormalMap);

        for ( unsigned int i = 0; i < 3; i++) {
            float spread = 4.5;
            glm::vec3 pos = testPositions[i] * spread;
            meshRenderer.translate(pos);
            meshRenderer.Draw(&camera);
        }
        groundRenderer.Draw(&camera);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // apply SSAO + honestly use this for SSR too?
        if (showSSAO)
        {
            glViewport(0, 0, WIDTH / 2, HEIGHT / 2);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, SSAOfbo);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);

            // use SSAO shader
            SSAOShader.use();
            SSAOShader.setMat4("projection", camera.getProjectionMatrix());
            SSAOShader.setMat4("view", camera.getViewMatrix());
            SSAOShader.setInt("samples", SSAOsamples);
            SSAOShader.setFloat("radius", SSAOradius);
            SSAOShader.setFloat("aoStrength", SSAOstrength);
            SSAOShader.setFloat("sssStrength", sss_strength);
            envMap.renderQuad();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // TO DO: Refactor this, this should be a function called blur() or something eventually
            // PASS 1
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, blurfbo);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, SSAObuffer);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gaussKernel.getTextureID());
            
            BlurShader.use();
            BlurShader.setBool("isAO", true);
            BlurShader.setInt("u_PingPong", 0);
            envMap.renderQuad();

            // PASS 2
            // rewrite to SSAO fbo
            glBindFramebuffer(GL_FRAMEBUFFER, SSAOfbo);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // re-assign sample texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, blurbuffer);

            BlurShader.use();
            BlurShader.setBool("isAO", true);
            BlurShader.setInt("u_PingPong", 1);
            envMap.renderQuad();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            glViewport(0, 0, WIDTH, HEIGHT);
        }

        // use deferred lighting shader
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw skybox
        if (showEnv)
        {
            glDepthMask(GL_FALSE);
            envMap.draw(&camera);
            glDepthMask(GL_TRUE);
        }

        deferredShader.use();
        deferredShader.setVec3("u_CamPos", camera.eye);
        deferredShader.setBool("u_EnableSSAO", showSSAO);
        deferredShader.setBool("u_DebugSSAO", showSSAODebug);
        deferredShader.setFloat("aoVal", u_AmbientOcclusion);
        deferredShader.setVec3("u_SSSColor", SSSColor);
        deferredShader.setFloat("u_Distortion", sss_distortion);
        deferredShader.setFloat("u_Scale", sss_scale);
        deferredShader.setFloat("u_Ambient", sss_ambient);
        deferredShader.setFloat("u_Glow", sss_glow);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, gMaterial);
        // assign diffuse map
        // assign glossy map
        // assign BRDF_LUT map
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.getIrradianceMap());
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.getSpecularMap());
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, envMap.getBRDFLUT());
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, SSAObuffer);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        envMap.renderQuad();
        glDisable(GL_BLEND);

        // some combination final pass to combine SSAO and our deferred render
        //testQuad.use();
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, SSAObuffer);
        //envMap.renderQuad();

        ImGui::Begin("Settings");
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        if (ImGui::CollapsingHeader("PBR Settings"))
        {
            ImGui::ColorEdit3("Albedo", color);
            ImGui::SliderFloat("Roughness", &u_Roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &u_Metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("AO", &u_AmbientOcclusion, 0.0f, 1.0f);
            ImGui::Checkbox("Albedo Map", &useAlbedoMap);
            ImGui::Checkbox("Normal Map", &useNormalMap);
            ImGui::Checkbox("Metallic Map", &useMetallicMap);
            ImGui::Checkbox("Roughness Map", &useRoughnessMap);
        }

        if (ImGui::CollapsingHeader("Display Settings"))
        {
            ImGui::Checkbox("Show model", &showModel);
            ImGui::Checkbox("Turntable", &turntable);
            ImGui::Checkbox("Angled Turn", &angledTurn);
        }

        if (ImGui::CollapsingHeader("Background Settings"))
        {
            ImGui::Checkbox("Show environment", &showEnv);
            ImGui::ColorEdit3("Background", background);
        }

        if (ImGui::CollapsingHeader("Deferred Outputs"))
        {
            ImGui::Image((void*)(intptr_t) gPosition, ImVec2(160,100), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t) gNormal, ImVec2(160,100), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

            ImGui::Image((void*)(intptr_t) gAlbedo, ImVec2(160,100), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t) gMaterial, ImVec2(160,100), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        }

        if (ImGui::CollapsingHeader("SSAO Settings"))
        {
            ImGui::Checkbox("Enable SSAO", &showSSAO);
            ImGui::SliderInt("Samples", &SSAOsamples, 0, 40);
            ImGui::SliderFloat("Radius", &SSAOradius, 0.0f, 1.0f);
            ImGui::SliderFloat("Strength", &SSAOstrength, 0.0f, 2.5f);
            ImGui::Checkbox("SSAO Debug", &showSSAODebug);
        }

        if (ImGui::CollapsingHeader("SSS Settings"))
        {
            ImGui::ColorEdit3("SSS Color", sssColor);
            ImGui::SliderFloat("Distortion", &sss_distortion, 0.0f, 2.0f);
            ImGui::SliderFloat("Scale", &sss_scale, 0.0f, 20.f);
            ImGui::SliderFloat("Ambient", &sss_ambient, 0.0f, 2.0f);
            ImGui::SliderFloat("Glow", &sss_glow, 0.0f, 2.0f);
            ImGui::SliderFloat("Inv. Strength", &sss_strength, 0.0f, 1.0f);
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        u_Albedo = glm::vec3(color[0], color[1], color[2]);
        SSSColor = glm::vec3(sssColor[0], sssColor[1], sssColor[2]);

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // imgui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // clean up
    glfwTerminate();
}

void setupGBuffer()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // generate position buffer -- id: 0
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // generate normal buffer -- id: 1
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // generate albedo buffer -- id: 2
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

    // generate material buffer -- id: 3
    glGenTextures(1, &gMaterial);
    glBindTexture(GL_TEXTURE_2D, gMaterial);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMaterial, 0);

    // write to four textures, but I'm going to be honest we probably don't need the fourth one
    GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    glGenRenderbuffers(1, &gDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupSSAO() // + thickness
{
    glGenFramebuffers(1, &SSAOfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, SSAOfbo);

    glGenTextures(1, &SSAObuffer);
    glBindTexture(GL_TEXTURE_2D, SSAObuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, WIDTH / 2, HEIGHT / 2, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAObuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    // worry about blurring and all of those fancy techniques later

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupBlur()
{
    glGenFramebuffers(1, &blurfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, blurfbo);

    glGenTextures(1, &blurbuffer);
    glBindTexture(GL_TEXTURE_2D, blurbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH / 2, HEIGHT / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurbuffer, 0);

    // GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
    // glDrawBuffers(1, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;

    std::cout << "blur setup complete" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}