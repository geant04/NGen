#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <GLFW/glfw3.h>

class Camera
{
public:
    Camera();
    Camera(unsigned int width, unsigned int height);
    Camera(unsigned int width, unsigned int height, glm::vec3 eye, glm::vec3 ref, glm::vec3 up);
    ~Camera();

    void setWidth(unsigned int width){
        this->width = width;
    }

    void setHeight(unsigned int height){
        this->height = height;
    }

    unsigned int getWidth(){
        return width;
    }

    unsigned int getHeight(){
        return height;
    }

    float fov = 45.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    glm::vec3 eye, // camera position
              ref, // target
              look, // look vector
              up,
              right,
              world_up, // global up
              V,
              H; 

    glm::mat4 getViewProjectionMatrix();
    glm::mat4 getProjectionMatrix();
    glm::mat4 getViewMatrix();

    void zoom(float amt) {
        eye += look * amt;
    }

    void translateRight(float amt) {
        eye += right * amt;
        ref += right * amt;
    }

    void translateUp(float amt) {
        eye += up * amt;
        ref += up * amt;
    }

    void translateForward(float amt) {
        eye += look * amt;
        ref += look * amt;
    }

    void processKeyboard(GLFWwindow* window, float deltaTime);

private:
    unsigned int width;
    unsigned int height;
};