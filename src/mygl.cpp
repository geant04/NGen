#include "mygl.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "mesh.h"

#include <glm/gtc/matrix_transform.hpp>

MyGL::MyGL()
{
    init();
}

MyGL::~MyGL()
{
    // clean up
}

// camera
const int WIDTH = 800;
const int HEIGHT = 600;
Camera camera(800, 600);

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

    camera.processKeyboard(window, deltaTime);
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

    // shader program
    Shader ourShader("../shaders/vert.glsl", "../shaders/frag.glsl");

    std::vector<glm::vec3> pos {glm::vec3(-1, -1, -1),
                                glm::vec3( 1, -1, -1),
                                glm::vec3( 1,  1, -1),
                                glm::vec3(-1,  1, -1),
                                glm::vec3(-1, -1,  1),
                                glm::vec3( 1, -1,  1),
                                glm::vec3( 1,  1,  1),
                                glm::vec3(-1,  1,  1)};

    std::vector<GLuint> idx {1, 0, 3, 1, 3, 2,
                             4, 5, 6, 4, 6, 7,
                             5, 1, 2, 5, 2, 6,
                             7, 6, 2, 7, 2, 3,
                             0, 4, 7, 0, 7, 3,
                             0, 1, 5, 0, 5, 4};

    std::vector<Vertex> vertices {};

    std::vector<unsigned int> indices {};

    for (unsigned int i = 0; i < 8; i++)
    {
        Vertex v;
        v.Position = 0.5f * pos[i];
        v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        v.TexCoords = glm::vec2(0.0f);
        vertices.push_back(v);
    }

    for (unsigned int i = 0; i < 36; i++)
    {
        indices.push_back(idx[i]);
    }

    Mesh ourTriangle(vertices, indices, std::vector<Texture>());

    //Camera camera(WIDTH, HEIGHT);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

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
        model = glm::rotate(model, currentFrame * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

        glm::mat4 view = glm::mat4(1.0f);
        view = camera.getViewMatrix();

        glm::mat4 projection = glm::mat4(1.0f);
        projection = camera.getProjectionMatrix();

        // set some stuff
        ourShader.use();
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);

        ourTriangle.Draw();

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // clean up
    glfwTerminate();
}

