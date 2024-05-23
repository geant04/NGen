#include "mygl.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "mesh.h"
#include "skybox.h"
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

MyGL::MyGL()
{
    init();
}

MyGL::~MyGL()
{
    // clean up
}

// camera
const int WIDTH = 1200;
const int HEIGHT = 900;
Camera camera(WIDTH, HEIGHT);
glm::vec2 lastMousePos = glm::vec2(WIDTH / 2, HEIGHT / 2);
bool firstMouse = true;
bool mousePressed = false;
bool wantCapture = false;

// display settings
bool turntable = false;
bool angledTurn = false;

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

void MyGL::init() 
{
    // glfw initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // window creation
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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // load our scene
    bool pbr = true;

    Shader ourShader;

    // skybox time
    std::string skyboxName = "betterSkybox";
    std::vector<std::string> faces {
        "textures/skybox/" + skyboxName + "/right.jpg",
        "textures/skybox/" + skyboxName + "/left.jpg",
        "textures/skybox/" + skyboxName + "/top.jpg",
        "textures/skybox/" + skyboxName + "/bottom.jpg",
        "textures/skybox/" + skyboxName + "/front.jpg",
        "textures/skybox/" + skyboxName + "/back.jpg"
    };
    //Skybox skybox;
    //skybox.loadCubemap(faces);
    //ourMesh.bindCubeMap(skybox.getCubemap());

    Texture2D albedoMap;
    Texture2D normalMap;
    Texture2D metallicMap;
    Texture2D roughnessMap;

    // constants for the shader
    glm::vec3 u_Albedo = glm::vec3(0.5f, 0.0f, 0.0f);
    float u_Roughness = 0.5f;
    float u_Metallic = 0.5f;
    float u_AmbientOcclusion = 1.0f;
    float color[3] = {u_Albedo.r, u_Albedo.g, u_Albedo.b};
    float background[3] = {0.0f, 0.0f, 0.0f};

    bool useAlbedoMap = true;
    bool useNormalMap = true;
    bool useMetallicMap = true;
    bool useRoughnessMap = true;

    Skybox envMap;
    envMap.loadCubemap(faces);
    bool showEnv = true;

    if (pbr) {
        ourShader = Shader("shaders/pbr/pbrvert.glsl", "shaders/pbr/pbrfrag.glsl");
        ourShader.use();
        ourShader.setInt("u_AlbedoMap", 3);
        ourShader.setInt("u_NormalMap", 4);
        ourShader.setInt("u_MetallicMap", 5);
        ourShader.setInt("u_RoughnessMap", 6);

        ourShader.setBool("u_UseAlbedoMap", useAlbedoMap);
        ourShader.setBool("u_UseNormalMap", useNormalMap);
        ourShader.setBool("u_UseMetallicMap", useMetallicMap);
        ourShader.setBool("u_UseRoughnessMap", useRoughnessMap);

        //load textures
        // albedoMap.loadTexture("textures/pbr/rustediron2_basecolor.png");
        // normalMap.loadTexture("textures/pbr/rustediron2_normal.png");
        // metallicMap.loadTexture("textures/pbr/rustediron2_metallic.png");
        // roughnessMap.loadTexture("textures/pbr/rustediron2_roughness.png");

        //albedoMap.loadTexture("textures/pbrCopper/Copper-scuffed_basecolor-boosted.png");
        // normalMap.loadTexture("textures/pbrCopper/Copper-scuffed_normal.png");
        // metallicMap.loadTexture("textures/pbrCopper/Copper-scuffed_metallic.png");
        // roughnessMap.loadTexture("textures/pbrCopper/Copper-scuffed_roughness.png");

        // albedoMap.loadTexture("textures/cync/cazas_texture.png");
        // normalMap.loadTexture("textures/pbrCopper/Copper-scuffed_normal.png");
        // metallicMap.loadTexture("textures/pbrCopper/Copper-scuffed_metallic.png");
        // roughnessMap.loadTexture("textures/pbrCopper/Copper-scuffed_roughness.png");

        albedoMap.loadTexture("textures/pbrWood/mahogfloor_basecolor.png");
        normalMap.loadTexture("textures/pbrWood/mahogfloor_normal.png");
        roughnessMap.loadTexture("textures/pbrWood/mahogfloor_roughness.png");

        //envMap.loadHDR("textures/hdr/hangar_interior_4k.hdr");
        //senvMap.loadCubemap(faces);

        // glm::mat4 projection = camera.getProjectionMatrix();
        // ourShader.use();
        // pbrShader.setMat4("projection", projection);
        
        // backgroundShader.use();
        // backgroundShader.setMat4("projection", projection);

        // then before rendering, configure the viewport to the original framebuffer's screen dimensions
        // int scrWidth, scrHeight;
        // glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
        // glViewport(0, 0, scrWidth, scrHeight);

    } else {
        ourShader = Shader("shaders/basic/vert.glsl", "shaders/basic/frag.glsl");
        ourShader.setInt("envMap", 0);
        //envMap.loadCubemap(faces);
    }

    // this shader is for simple rendering
    //Shader ourShader("../shaders/vert.glsl", "../shaders/frag.glsl");
    // this shader is for PBR rendering
    //Shader ourShader("../shaders/pbrvert.glsl", "../shaders/pbrfrag.glsl");

    Mesh ourMesh;
    std::string modelName = "teapot";
    std::string modelPath = "models/" + modelName + ".obj";
    ourMesh.LoadObj(modelPath.c_str());
    ourMesh.create();
    bool showModel = true;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // iniitalize imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, camera, deltaTime);

        // rendering commands here
        glEnable(GL_DEPTH_TEST);  
        glClearColor(background[0], background[1], background[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        wantCapture = io.WantCaptureMouse;

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // camera
        glm::mat4 model = glm::mat4(1.0f);
        
        // teapot is a bit big
        if (!modelName.compare("teapot"))
        {
            model = glm::scale(model, glm::vec3(0.15f));
        }

        if (!modelName.compare("itme"))
        {
            model = glm::scale(model, glm::vec3(0.1f));
        }

        glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
        float theta = glm::radians(50.0f);

        if (angledTurn) {
            axis.x = 0.2f;
        }

        if (turntable) {
            theta *= currentFrame;
        }

        if (angledTurn || turntable)
        {
            model = glm::rotate(model, theta, axis);
        }

        glm::mat4 view = glm::mat4(1.0f);
        view = camera.getViewMatrix();

        glm::mat4 projection = glm::mat4(1.0f);
        projection = camera.getProjectionMatrix();

        // set some stuff
        ourShader.use();
        ourShader.setMat4("model", model);
        ourShader.setMat4("modelInvTrans", glm::transpose(glm::inverse(model)));
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("u_CamPos", camera.eye);
        ourShader.setVec3("u_Albedo", u_Albedo);
        ourShader.setFloat("u_Roughness", u_Roughness);
        ourShader.setFloat("u_Metallic", u_Metallic);
        ourShader.setFloat("u_AmbientOcclusion", u_AmbientOcclusion);

        ourShader.setBool("u_UseAlbedoMap", useAlbedoMap);
        ourShader.setBool("u_UseNormalMap", useNormalMap);
        ourShader.setBool("u_UseRoughnessMap", useRoughnessMap);
        ourShader.setBool("u_UseMetallicMap", useMetallicMap);

        if (pbr)
        {
            // bind textures for PBR
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, albedoMap.getTextureID());
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, normalMap.getTextureID());
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, metallicMap.getTextureID());
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, roughnessMap.getTextureID());
            // there should be a slot for AO map but it's simply white
        }
        
        if (showModel) 
        {
            ourMesh.Draw();
        }

        // render the skybox
        //skybox.draw(view, projection);
        if (showEnv)
        {
            envMap.draw(view, projection);
        }

        ImGui::Begin("Settings");
        ImGui::Text("PBR Settings");
        ImGui::ColorEdit3("Albedo", color);
        ImGui::SliderFloat("Roughness", &u_Roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("Metallic", &u_Metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Ambient Occlusion", &u_AmbientOcclusion, 0.0f, 1.0f);
        ImGui::Checkbox("Albedo Map", &useAlbedoMap);
        ImGui::Checkbox("Normal Map", &useNormalMap);
        ImGui::Checkbox("Metallic Map", &useMetallicMap);
        ImGui::Checkbox("Roughness Map", &useRoughnessMap);
        ImGui::Text("Display Settings");
        ImGui::Checkbox("Show model", &showModel);
        ImGui::Checkbox("Turntable", &turntable);
        ImGui::Checkbox("Angled Turn", &angledTurn);
        ImGui::Text("Background Settings");
        ImGui::Checkbox("Show environment", &showEnv);
        ImGui::ColorEdit3("Background", background);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        u_Albedo = glm::vec3(color[0], color[1], color[2]);

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

