#include "camera.h"
#include <iostream>

Camera::Camera() :
    Camera(400, 400)
{}

Camera::Camera(unsigned int width, unsigned int height) 
    : fov(55), near(0.1), far(1000),
      eye(0,0,10), ref(0,0,0),
      look(0,0,-1), up(0,1,0), right(glm::cross(look, up)),
      width(width), height(height), world_up(0, 1, 0)
{
    std::cout << "Camera created" << std::endl;
    std::cout << "far: "  << far << std::endl;
    std::cout << "near: " << near << std::endl;
}

Camera::~Camera()
{
    // clean up
}

glm::mat4 Camera::getViewProjectionMatrix() const
{
    return getProjectionMatrix() * getViewMatrix();
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov), (float)width / (float)height, near, far);
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(eye, ref, up);
}