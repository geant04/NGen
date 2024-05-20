#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

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

    void recomputeAttributes() {
        look = glm::normalize(ref - eye);
        right = glm::normalize(glm::cross(look, up));
        up = glm::normalize(glm::cross(right, look));
    }

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

    void rotateRight(float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, world_up);
        look = glm::vec3(rotation * glm::vec4(look, 1.0));
        right = glm::vec3(rotation * glm::vec4(right, 1.0));     
        ref = eye + look;
    }

    void rotateUp(float angle) {
        glm::vec3 axis = glm::normalize(glm::cross(look, world_up));
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
        look = glm::vec3(rotation * glm::vec4(look, 1.0));
        up = glm::vec3(rotation * glm::vec4(up, 1.0));
        right = glm::normalize(glm::cross(look, up));
        ref = eye + look;
    }

    // thanks mally
    void rotateTheta(float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), right);
        eye = eye - ref;
        eye = glm::vec3(rotation * glm::vec4(eye, 1.0));
        eye = eye + ref;
        recomputeAttributes();
    }

    void rotatePhi(float angle) {
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), up);
        eye = eye - ref;
        eye = glm::vec3(rotation * glm::vec4(eye, 1.0));
        eye = eye + ref;
        recomputeAttributes();
    }

private:
    unsigned int width;
    unsigned int height;
};