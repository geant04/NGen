#include "mygl.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "mesh.h"
#include "skybox.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

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

// display settings
bool turntable = true;
bool angledTurn = true;

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

    if (mousePressed)
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

    bool pbr = false;

    Shader ourShader;

    if (pbr) {
        ourShader = Shader("../shaders/pbr/pbrvert.glsl", "../shaders/pbr/pbrfrag.glsl");
    } else {
        ourShader = Shader("../shaders/basic/vert.glsl", "../shaders/basic/frag.glsl");
        ourShader.setInt("envMap", 0);
    }

    // this shader is for simple rendering
    //Shader ourShader("../shaders/vert.glsl", "../shaders/frag.glsl");
    // this shader is for PBR rendering
    //Shader ourShader("../shaders/pbrvert.glsl", "../shaders/pbrfrag.glsl");

    Mesh ourMesh;
    ourMesh.LoadObj("../models/teapot.obj");
    ourMesh.create();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // constants for the shader
    glm::vec3 u_Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
    float u_Roughness = 0.2f;
    float u_Metallic = 0.2f;
    float u_AmbientOcclusion = 1.0f;

    // skybox time
    std::string skyboxName = "betterSkybox";
    std::vector<std::string> faces {
        "../textures/skybox/" + skyboxName + "/right.jpg",
        "../textures/skybox/" + skyboxName + "/left.jpg",
        "../textures/skybox/" + skyboxName + "/top.jpg",
        "../textures/skybox/" + skyboxName + "/bottom.jpg",
        "../textures/skybox/" + skyboxName + "/front.jpg",
        "../textures/skybox/" + skyboxName + "/back.jpg"
    };
    Skybox skybox;
    skybox.loadCubemap(faces);
    ourMesh.bindCubeMap(skybox.getCubemap());

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, camera, deltaTime);

        // rendering commands here
        glEnable(GL_DEPTH_TEST);  
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // camera
        glm::mat4 model = glm::mat4(1.0f);
        
        // teapot is a bit big
        model = glm::scale(model, glm::vec3(0.10f));

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

        ourMesh.Draw();

        // render the skybox
        skybox.draw(view, projection);

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // clean up
    glfwTerminate();
}

