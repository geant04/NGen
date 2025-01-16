cmake_minimum_required(VERSION 3.14)

# Set the project name and version
project(MyProgram VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the runtime output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set the paths for external libraries
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/lib)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/include/imgui)
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/include/glad)

# Add the include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${GLAD_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(APPEND SOURCES
    ${CMAKE_SOURCE_DIR}/src/tiny_obj_loader.cc
    ${CMAKE_SOURCE_DIR}/src/glad.c
)

file(GLOB IMGUI_SOURCES "${IMGUI_DIR}/*.cpp")
list(APPEND SOURCES ${IMGUI_SOURCES})

# Create the executable
add_executable(myprogram ${SOURCES})

find_package(OpenGL REQUIRED)
# Link GLFW and OpenGL libraries
target_link_libraries(myprogram
    ${GLFW_DIR}/libglfw3dll.a  # Dynamic GLFW (replace with .lib for MSVC or static .a)
    OpenGL::GL  # OpenGL (MinGW should have this)
)

# For MSYS2/MinGW: Set additional flags
if(MINGW)
    set_target_properties(myprogram PROPERTIES LINK_FLAGS "-lglfw3dll")
endif()

# Copy glfw3.dll to the output directory after building
add_custom_command(TARGET myprogram POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/glfw3.dll
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/glfw3.dll
)

add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/bin/Debug/myprogram.exe
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)