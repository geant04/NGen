#include "mygl.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "mesh.h"

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

    // load function pointers??
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    // load our scene
    //Shader ourShader("../shaders/vert.glsl", "../shaders/frag.glsl");
    Shader ourShader("../shaders/pbrvert.glsl", "../shaders/pbrfrag.glsl");
    Mesh ourMesh;
    ourMesh.LoadObj("../models/tahu.obj");
    ourMesh.create();

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // constants for the shader
    glm::vec3 u_Albedo = glm::vec3(0.5f, 0.0f, 0.0f);
    float u_Roughness = 0.2f;
    float u_Metallic = 0.5f;
    float u_AmbientOcclusion = 1.0f;

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
        model = glm::scale(model, glm::vec3(0.5f));
        model = glm::rotate(model, currentFrame * glm::radians(50.0f), glm::vec3(0.25f, 1.0f, 0.0f));

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

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // clean up
    glfwTerminate();
}

