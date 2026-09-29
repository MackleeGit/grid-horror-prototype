cmake_minimum_required(VERSION 3.15)

# ------------------------------------------------------------------
# Project definition
# ------------------------------------------------------------------
# Enable both C and C++ – we need C for glad.c
project(EchoesInTheDark LANGUAGES C CXX)


# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------
# Find required packages
# ------------------------------------------------------------------
find_package(GLFW3 REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)

# ------------------------------------------------------------------
# GLAD – OpenGL function loader
# ------------------------------------------------------------------
# Header location
include_directories(${PROJECT_SOURCE_DIR}/external/glad/include)

# Source files
# 1) All your own C++ source files (e.g. src/main.cpp)
file(GLOB_RECURSE CPP_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 2) The GLAD implementation file (C source)
set(GLAD_SOURCE "${PROJECT_SOURCE_DIR}/external/glad/src/glad.c")

# 3) Combine both lists for the executable
set(ALL_SOURCES ${CPP_SOURCES} ${GLAD_SOURCE})

# ------------------------------------------------------------------
# Build target
# ------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# ------------------------------------------------------------------
# Link libraries
# ------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glfw          # GLFW runtime
        glm::glm      # GLM math library
        OpenGL::GL)   # System OpenGL library (opengl32 on Windows)
