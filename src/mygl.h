#ifndef MYGL_H
#define MYGL_H

#include "camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class MyGL
{
public:
    MyGL();
    ~MyGL();

    void init();

private:
    // camera

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    bool firstMouse;
    bool wantCapture;
    glm::vec2 lastMousePos;
    Camera camera;

    // display settings
    bool turntable = false;
    bool angledTurn = false;
    bool showSSAO = true;

    void handleMouseCallBack();
};

#endif