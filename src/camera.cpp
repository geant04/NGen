#include "camera.h"
#include <iostream>

Camera::Camera() :
    Camera(400, 400)
{}

Camera::Camera(unsigned int width, unsigned int height) 
    : Camera(width, height, glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))
{
    look = glm::normalize(ref - eye);
    right = glm::normalize(glm::cross(look, up));
    world_up = glm::vec3(0.0f, 1.0f, 0.0f);
}

Camera::Camera(unsigned int width, unsigned int height, glm::vec3 eye, glm::vec3 ref, glm::vec3 up) 
 : width(width), height(height), eye(eye), ref(ref), up(up), near(0.1), far(1000.0), fov(45.0)
{
    std::cout << "Camera created" << std::endl;
}

Camera::~Camera()
{
    // clean up
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov), (float)width / (float)height, near, far);
}

glm::mat4 Camera::getViewProjectionMatrix() const
{
    return getProjectionMatrix() * glm::lookAt(eye, ref, up);
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(this->eye,   // Camera position in world space
                       this->ref,   // Looking at the origin
                       this->up);  // Up vector (positive Y-axis)
}