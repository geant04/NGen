#include "mygl.h"

#include "shader.h"
#include "mesh.h"
#include "skybox.h"
#include "geometry/quad.h"
#include "geometry/cube.h"
#include "features/ssr.h"
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

#include "deferred.h"
#include "SSAO.h"

# define M_PI           3.14159265358979323846

unsigned int WIDTH = 1280, HEIGHT = 720;
glm::vec2 mousePos;
bool mousePressed;

MyGL::MyGL()
{
    camera = Camera(WIDTH, HEIGHT);
    lastMousePos = glm::vec2(WIDTH / 2, HEIGHT / 2);
    firstMouse = true;
    init();
}

MyGL::~MyGL()
{
    // clean up
    glfwTerminate();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    WIDTH = width;
    HEIGHT = height;
}

void processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    float cameraSpeed = 5.0f;
    float rotationSpeed = cameraSpeed * cameraSpeed;
    cameraSpeed *= deltaTime;
    rotationSpeed *= deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.zoom(cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.zoom(-cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.translateRight(-cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.translateRight(cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.translateUp(-cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.translateUp(cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.rotatePhi(-rotationSpeed);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.rotatePhi(rotationSpeed);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.rotateTheta(-rotationSpeed);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.rotateTheta(rotationSpeed);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    mousePos = glm::vec2(xpos, ypos);
}

void MyGL::handleMouseCallBack()
{
    if (firstMouse)
    {
        lastMousePos = mousePos;
        firstMouse = false;
    }

    glm::vec2 offset = mousePos - lastMousePos;
    lastMousePos = mousePos;

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
        mousePressed = true;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        mousePressed = false;
    }
}

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return;
    }

    // register callback -- handle window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    camera.setWidth(WIDTH); // needs to recalculate the projection matrix?
    camera.setHeight(HEIGHT);

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
    float yDisplacement = 0; //-0.4;

    MeshRenderer meshRenderer;
    meshRenderer.SetMesh(&ourMesh);
    meshRenderer.LoadMaterials("textures/jade/jade_albedo.jpg",
                               "textures/pbrWood/mahogfloor_normal.png",
                               "textures/pbrGold/gold-scuffed_metallic.png",
                               "textures/pbrGold/gold-scuffed_roughness.png");
    meshRenderer.LoadShader("shaders/deferred/gbuffer.vert.glsl", "shaders/deferred/gbuffer.frag.glsl");

    Cube groundMesh;
    groundMesh.create();

    MeshRenderer groundRenderer;
    Texture2D funWall = Texture2D("textures/funTextures/crab_rig.png");
    groundRenderer.SetMesh(&groundMesh);
    groundRenderer.LoadMaterials("textures/pbrCopper/Copper-scuffed_basecolor-boosted.png",
                               "textures/pbrCopper/Copper-scuffed_normal.png",
                               "textures/pbrCopper/Copper-scuffed_metallic.png",
                               "textures/pbrCopper/Copper-scuffed_roughness.png");
    groundRenderer.LoadShader("shaders/deferred/gbuffer.vert.glsl", "shaders/deferred/gbuffer.frag.glsl");
    groundRenderer.rotate(0, glm::vec3(0, 1, 0));
    groundRenderer.translate(glm::vec3(0, -4.75, 0));
    groundRenderer.scale(glm::vec3(8, 0.75, 6));
    groundRenderer.setMapToggles(true, true, true, true);

    Skybox envMap;
    envMap.loadHDR("textures/hdr/hangar_interior_4k.hdr");
    envMap.createIrradianceMap();
    envMap.createSpecularMap();
    envMap.createBRDFLUT();

    Quad quad;
    quad.create();

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

    // MeshRenderer params...
    bool showModel = true;
    bool showEnv = true;
    bool showSSAODebug = false;
    bool enableSSR = true;
    bool showSSRDebug = false;
    bool enableComposite = true;

    glm::vec3 SSSColor = glm::vec3(1.f, 1.0f, 1.0f);
    float sssColor[3] = {SSSColor.r, SSSColor.g, SSSColor.b};
    float sss_distortion = 0.352f;
    float sss_scale = 1.0f;
    float sss_ambient = 0.2f;
    float sss_glow = 2.0f;
    float sss_strength = 1.0f;

    // render loop
    // --------------------------

    // Deferred rendering setup
    DeferredFramebuffer deferred;
    deferred.Create(WIDTH, HEIGHT);
    unsigned int gBuffer = deferred.GetGBuffer();

    // SSAO setup
    SSAO ssao = SSAO();
    ssao.Create(WIDTH, HEIGHT, true);

    // SSR setup
    SSR ssr = SSR(WIDTH, HEIGHT);

    const glm::vec3 testPositions[9] = {
        glm::vec3( 0., yDisplacement, 0.),
        glm::vec3(-1, yDisplacement, 0.),
        glm::vec3( 1., yDisplacement, 0.),
        glm::vec3(-1., yDisplacement, 1.),
        glm::vec3( 0., yDisplacement, 1.),
        glm::vec3( 1., yDisplacement, 1.),
        glm::vec3(-1., yDisplacement, -1.),
        glm::vec3( 0., yDisplacement, -1.),
        glm::vec3( 1., yDisplacement, -1.)
    };

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, camera, deltaTime);
        handleMouseCallBack();

        // frame logic
        float currentFrame = float(glfwGetTime());
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
        //meshRenderer.setMapToggles(useAlbedoMap, useRoughnessMap, useMetallicMap, useNormalMap);

        for (unsigned int i = 0; i < 1; i++) {
            meshRenderer.translate(testPositions[i] * 4.5f);
            meshRenderer.Draw(&camera);
        }

        // render ground and wall...
        groundRenderer.translate(glm::vec3(0, -4.75, 0));
        groundRenderer.scale(glm::vec3(8, 0.75, 8));
        groundRenderer.Draw(&camera); // ground

        groundRenderer.translate(glm::vec3(0, 4, -7));
        groundRenderer.scale(glm::vec3(8.0, 8.0, 0.75));
        groundRenderer.Draw(
            &camera,
            funWall.getTextureID(),
            groundRenderer.getMetallicID(),
            groundRenderer.getNormalID(),
            groundRenderer.getRoughnessID()
        ); // wall

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // apply SSAO + honestly use this for SSR too?
        if (showSSAO)
        {
            // SSAO pass
            ssao.SSAOPass(deferred.GetGPosition(), deferred.GetGNormal(), camera, quad);
        }

        // temporary SSR pass placement
        if (enableSSR && false)
        {
            // linker not set up properly, to do: move ssr.h implementation contents to ssr.cpp
            ssr.SSRPass(
                deferred.GetGPosition(), 
                deferred.GetGNormal(), 
                deferred.GetGAlbedo(),
                deferred.GetGMaterial(),
                camera,
                quad);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw skybox
        if (showEnv)
        {
            glDepthMask(GL_FALSE);
            envMap.draw(&camera);
            glDepthMask(GL_TRUE);
        }
        
        // use deferred lighting shader
        if (enableComposite)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            Shader* deferredShader = deferred.GetShaderPointer();
            deferredShader->use();
            deferredShader->setVec3("u_CamPos", camera.eye); // pass in Camera, pass in a struct called ImGui settings...
            deferredShader->setVec3("u_SSSColor", SSSColor);
            deferredShader->setBool("u_EnableSSAO", showSSAO);
            deferredShader->setBool("u_DebugSSAO", showSSAODebug);
            deferredShader->setFloat("aoVal", u_AmbientOcclusion);
            deferredShader->setFloat("u_Distortion", sss_distortion);
            deferredShader->setFloat("u_Scale", sss_scale);
            deferredShader->setFloat("u_Ambient", sss_ambient);
            deferredShader->setFloat("u_Glow", sss_glow);
            deferredShader->setInt("gPosition", 0);
            deferredShader->setInt("gNormal", 1);
            deferredShader->setInt("gAlbedo", 2);
            deferredShader->setInt("gMaterial", 3);
            deferredShader->setInt("u_IrradianceMap", 4);
            deferredShader->setInt("u_SpecularMap", 5);
            deferredShader->setInt("u_BRDFLUT", 6);
            deferredShader->setInt("u_SSAO", 7);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, deferred.GetGPosition());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, deferred.GetGNormal());
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, deferred.GetGAlbedo());
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, deferred.GetGMaterial());
            
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.getIrradianceMap());
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.getSpecularMap());
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, envMap.getBRDFLUT());
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, ssao.GetSSAOBuffer());

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            quad.Draw();
            glDisable(GL_BLEND);
        }

        ImGui::Begin("Settings");
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        if (ImGui::CollapsingHeader("PBR Settings"))
        {
            ImGui::ColorEdit3("Albedo", color);
            ImGui::SliderFloat("Roughness", &u_Roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &u_Metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("AO", &u_AmbientOcclusion, 0.0f, 1.0f);
            ImGui::Checkbox("Albedo Map", &meshRenderer.useAlbedoMap);
            ImGui::Checkbox("Normal Map", &meshRenderer.useNormalMap);
            ImGui::Checkbox("Metallic Map", &meshRenderer.useMetallicMap);
            ImGui::Checkbox("Roughness Map", &meshRenderer.useRoughnessMap);
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
            ImGui::Image((void*)(intptr_t) deferred.GetGPosition(), ImVec2(160,100), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t) deferred.GetGNormal(), ImVec2(160,100), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

            ImGui::Image((void*)(intptr_t) deferred.GetGAlbedo(), ImVec2(160,100), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            ImGui::SameLine();
            ImGui::Image((void*)(intptr_t) deferred.GetGMaterial(), ImVec2(160,100), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        }

        if (ImGui::CollapsingHeader("SSAO Settings"))
        {
            ImGui::Checkbox("Show SSAO", &showSSAO);
            ImGui::SliderInt("Samples", &ssao.SSAOsamples, 0, 100);
            ImGui::SliderFloat("Radius", &ssao.SSAOradius, 0.0f, 1.0f);
            ImGui::SliderFloat("Strength", &ssao.SSAOstrength, 0.0f, 2.5f);
            ImGui::SliderFloat("Inv. Strength", &ssao.SSAOinvStrength, 0.0f, 1.0f);
            ImGui::Checkbox("SSAO Debug", &showSSAODebug);
        }

        if (ImGui::CollapsingHeader("SSS Settings"))
        {
            ImGui::ColorEdit3("SSS Color", sssColor);
            ImGui::SliderFloat("Distortion", &sss_distortion, 0.0f, 2.0f);
            ImGui::SliderFloat("Scale", &sss_scale, 0.0f, 20.f);
            ImGui::SliderFloat("Ambient", &sss_ambient, 0.0f, 2.0f);
            ImGui::SliderFloat("Glow", &sss_glow, 0.0f, 2.0f);
        }

        if (ImGui::CollapsingHeader("SSR Settings"))
        {
            ImGui::Checkbox("Enable SSR", &enableSSR);
            ImGui::SliderFloat("Thickness", &ssr.thickness, 0.0f, 1.0f);
            ImGui::SliderFloat("Max Distance", &ssr.maxDistance, 0.0f, 20.0f);
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