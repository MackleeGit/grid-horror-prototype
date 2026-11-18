
#include "../checkpoints/checkpoint1/cp1Geometry.h"
#include "../external/stb_image.h"
#include "Shader.hpp"
#include "algorithms/MinimapHUD.hpp"
#include "gameplay/InteractionSystem.hpp"
#include "gameplay/GameState.hpp"
#include "entity/MonsterAI.hpp"
#include "physics/CollisionEngine.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

// Screen and viewport defaults
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
int currentWidth = SCR_WIDTH;
int currentHeight = SCR_HEIGHT;

// Window and Fullscreen State
bool isFullscreen = false;
bool f11PressedLastFrame = false;
int windowedPosX = 100;
int windowedPosY = 100;
int windowedWidth = SCR_WIDTH;
int windowedHeight = SCR_HEIGHT;

// Pause and Cursor State
bool isPaused = false;
bool pausePressedLastFrame = false;

// Master level house layout
cp1::Grid houseGrid = cp1::defaultHouse();

// Gameplay systems
InteractionSystem interactionSystem;
MonsterAI monster;
GameState gameState = GameState::PLAYING;

// FPS Camera Representation
glm::vec3 cameraPos = glm::vec3(-7.0f, 0.0f, -5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Delta Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Input state
bool ePressedLastFrame = false;

// Movement state
bool isSprinting = false;
bool isCrouching = false;
bool isMoving = false;

// Debug Telemetry and HUD
bool showDebugTelemetry = false;
bool showMinimapHUD = true;
bool f3PressedLastFrame = false;
bool f2PressedLastFrame = false;

// Flashlight state
bool flashlightOn = true;
bool fPressedLastFrame = false;

// Framebuffer Callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (width > 0 && height > 0) {
        currentWidth = width;
        currentHeight = height;
        glViewport(0, 0, width, height);
    }
}

// Fullscreen Toggle
void toggleFullscreen(GLFWwindow* window) {
    isFullscreen = !isFullscreen;

    if (isFullscreen) {
        glfwGetWindowPos(window, &windowedPosX, &windowedPosY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        if (monitor) {
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            if (mode) {
                glfwSetWindowMonitor(
                    window, monitor, 0, 0,
                    mode->width, mode->height, mode->refreshRate
                );
            }
        }
    } else {
        glfwSetWindowMonitor(
            window, nullptr,
            windowedPosX, windowedPosY,
            windowedWidth, windowedHeight,
            0
        );
    }
}

// Mouse Button Callback
void mouse_button_callback(
    GLFWwindow* window,
    int button,
    int action,
    int mods
) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (isPaused) {
            isPaused = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
            glfwSetWindowTitle(window, "3D Survival Horror Application");
        }
    }
}

// Mouse Callback
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (isPaused ||
        gameState == GameState::CAUGHT ||
        gameState == GameState::ESCAPED) {
        return;
    }

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    pitch = std::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 direction;

    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(direction);
}

// Input Processing
void processInput(GLFWwindow* window) {

    // F11: Fullscreen Toggle
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
        if (!f11PressedLastFrame) {
            toggleFullscreen(window);
            f11PressedLastFrame = true;
        }
    } else {
        f11PressedLastFrame = false;
    }

    // ESC or P: Toggle Pause
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {

        if (!pausePressedLastFrame) {
            isPaused = !isPaused;
            pausePressedLastFrame = true;

            if (isPaused) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                glfwSetWindowTitle(
                    window,
                    "3D Survival Horror Application - [PAUSED]"
                );
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
                glfwSetWindowTitle(window, "3D Survival Horror Application");
            }
        }
    } else {
        pausePressedLastFrame = false;
    }

    // When paused, Q exits the application.
    if (isPaused) {
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        return;
    }

    // Stop gameplay input after a terminal state.
    if (gameState == GameState::CAUGHT ||
        gameState == GameState::ESCAPED) {
        return;
    }

    // F3: Toggle debug telemetry
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
        if (!f3PressedLastFrame) {
            showDebugTelemetry = !showDebugTelemetry;
            f3PressedLastFrame = true;
        }
    } else {
        f3PressedLastFrame = false;
    }

    // F2: Toggle minimap
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
        if (!f2PressedLastFrame) {
            showMinimapHUD = !showMinimapHUD;
            f2PressedLastFrame = true;
        }
    } else {
        f2PressedLastFrame = false;
    }

    // E: Interact with nearby fuse or closet
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!ePressedLastFrame) {
            interactionSystem.interact(cameraPos, houseGrid);
            ePressedLastFrame = true;
        }
    } else {
        ePressedLastFrame = false;
    }

    // F: Toggle flashlight
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fPressedLastFrame) {
            flashlightOn = !flashlightOn;
            fPressedLastFrame = true;
        }
    } else {
        fPressedLastFrame = false;
    }

    // Determine movement keys
    bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    bool movementInput = forward || backward || left || right;

    // Sprint and crouch cannot be active simultaneously.
    bool shiftPressed =
        glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    bool ctrlPressed =
        glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

    isSprinting = shiftPressed && !ctrlPressed && movementInput;
    isCrouching = ctrlPressed && movementInput;

    // Hiding prevents movement.
    isMoving = movementInput && !interactionSystem.isPlayerHidden;

    if (!interactionSystem.isPlayerHidden) {

        float movementSpeed = 2.5f;

        if (isSprinting) {
            movementSpeed = 4.5f;
        } else if (isCrouching) {
            movementSpeed = 1.2f;
        }

        float cameraSpeed = movementSpeed * deltaTime;

        glm::vec3 horizontalFront(
            cameraFront.x,
            0.0f,
            cameraFront.z
        );

        if (glm::length(horizontalFront) > 0.0f) {
            horizontalFront = glm::normalize(horizontalFront);
        }

        glm::vec3 rightDirection =
            glm::normalize(glm::cross(horizontalFront, cameraUp));

        glm::vec3 movement(0.0f);

        if (forward)
            movement += horizontalFront;

        if (backward)
            movement -= horizontalFront;

        if (left)
            movement -= rightDirection;

        if (right)
            movement += rightDirection;

        if (glm::length(movement) > 0.0f) {
            movement = glm::normalize(movement);

            glm::vec3 deltaMovement = movement * cameraSpeed;

            cameraPos = CollisionEngine::resolveMovement(
                cameraPos,
                deltaMovement,
                houseGrid
            );
        }
    }

    cameraPos.y = 0.0f;
}

int main() {

    // Register the three fuse objectives.
    interactionSystem.initialize(houseGrid);

    // Initialize monster patrol position.
    monster.initialize(1.0f, -5.0f);

    std::cout << "[MONSTER] Monster AI initialized.\n";

    // CP1 geometry demos
    cp1::runGeometryDemo();

    // Export static minimap evidence.
    {
        MinimapHUD demoHUD(200, 200);

        demoHUD.render(
            -7.0f,
            -5.0f,
            -45.0f,
            1.5f,
            houseGrid
        );

        demoHUD.canvas.savePNG(
            "checkpoints/checkpoint1/cp1_output/cp1_minimap_hud.png"
        );
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "3D Survival Horror Application",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Center window on primary monitor.
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    if (primaryMonitor) {
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

        if (mode) {
            int posX = (mode->width - static_cast<int>(SCR_WIDTH)) / 2;
            int posY = (mode->height - static_cast<int>(SCR_HEIGHT)) / 2;

            glfwSetWindowPos(window, posX, posY);
        }
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD Runtime Loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader sceneShader("shaders/scene.vert", "shaders/scene.frag");
    Shader hudShader("shaders/hud.vert", "shaders/hud.frag");

    // Wall cube geometry
    float vertices[] = {
        -0.5f,-0.5f,-0.5f,0.0f,0.0f,  0.5f,-0.5f,-0.5f,1.0f,0.0f,
         0.5f, 0.5f,-0.5f,1.0f,1.0f,  0.5f, 0.5f,-0.5f,1.0f,1.0f,
        -0.5f, 0.5f,-0.5f,0.0f,1.0f, -0.5f,-0.5f,-0.5f,0.0f,0.0f,

        -0.5f,-0.5f, 0.5f,0.0f,0.0f,  0.5f,-0.5f, 0.5f,1.0f,0.0f,
         0.5f, 0.5f, 0.5f,1.0f,1.0f,  0.5f, 0.5f, 0.5f,1.0f,1.0f,
        -0.5f, 0.5f, 0.5f,0.0f,1.0f, -0.5f,-0.5f, 0.5f,0.0f,0.0f,

        -0.5f, 0.5f, 0.5f,1.0f,0.0f, -0.5f, 0.5f,-0.5f,1.0f,1.0f,
        -0.5f,-0.5f,-0.5f,0.0f,1.0f, -0.5f,-0.5f,-0.5f,0.0f,1.0f,
        -0.5f,-0.5f, 0.5f,0.0f,0.0f, -0.5f, 0.5f, 0.5f,1.0f,0.0f,

         0.5f, 0.5f, 0.5f,1.0f,0.0f,  0.5f, 0.5f,-0.5f,1.0f,1.0f,
         0.5f,-0.5f,-0.5f,0.0f,1.0f,  0.5f,-0.5f,-0.5f,0.0f,1.0f,
         0.5f,-0.5f, 0.5f,0.0f,0.0f,  0.5f, 0.5f, 0.5f,1.0f,0.0f,

        -0.5f,-0.5f,-0.5f,0.0f,1.0f,  0.5f,-0.5f,-0.5f,1.0f,1.0f,
         0.5f,-0.5f, 0.5f,1.0f,0.0f,  0.5f,-0.5f, 0.5f,1.0f,0.0f,
        -0.5f,-0.5f, 0.5f,0.0f,0.0f, -0.5f,-0.5f,-0.5f,0.0f,1.0f,

        -0.5f, 0.5f,-0.5f,0.0f,1.0f,  0.5f, 0.5f,-0.5f,1.0f,1.0f,
         0.5f, 0.5f, 0.5f,1.0f,0.0f,  0.5f, 0.5f, 0.5f,1.0f,0.0f,
        -0.5f, 0.5f, 0.5f,0.0f,0.0f, -0.5f, 0.5f,-0.5f,0.0f,1.0f
    };

    unsigned int VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // Floor Geometry
    float floorVertices[] = {
        -8.5f,-0.5f,-6.5f,0.0f,12.0f,
         8.5f,-0.5f,-6.5f,16.0f,12.0f,
         8.5f,-0.5f, 6.5f,16.0f,0.0f,
        -8.5f,-0.5f, 6.5f,0.0f,0.0f
    };

    unsigned int floorIndices[] = {0, 1, 2, 2, 3, 0};

    unsigned int floorVAO, floorVBO, floorEBO;

    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(floorVertices),
        floorVertices,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(floorIndices),
        floorIndices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // HUD quad
    float hudQuadVertices[] = {
        0.45f,0.95f,0.0f,0.0f, 0.95f,0.95f,1.0f,0.0f,
        0.95f,0.45f,1.0f,1.0f, 0.45f,0.95f,0.0f,0.0f,
        0.95f,0.45f,1.0f,1.0f, 0.45f,0.45f,0.0f,1.0f
    };

    unsigned int hudVAO, hudVBO;

    glGenVertexArrays(1, &hudVAO);
    glGenBuffers(1, &hudVBO);

    glBindVertexArray(hudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hudVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(hudQuadVertices),
        hudQuadVertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
        (void*)(2 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // Texture setup
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;

    // 1. Wall Texture
    unsigned int wallTexture;

    glGenTextures(1, &wallTexture);
    glBindTexture(GL_TEXTURE_2D, wallTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char* wallData =
        stbi_load("assets/textures/wall.png", &width, &height, &nrChannels, 0);

    if (wallData) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(
            GL_TEXTURE_2D, 0, format, width, height, 0,
            format, GL_UNSIGNED_BYTE, wallData
        );

        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(wallData);
    } else {
        unsigned char brickPixels[16 * 16 * 4];

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int index = (y * 16 + x) * 4;

                bool isMortar =
                    (y % 4 == 0) ||
                    ((y / 4) % 2 == 0
                        ? (x % 8 == 0)
                        : ((x + 4) % 8 == 0));

                brickPixels[index + 0] = isMortar ? 40 : 95;
                brickPixels[index + 1] = isMortar ? 42 : 55;
                brickPixels[index + 2] = isMortar ? 45 : 50;
                brickPixels[index + 3] = 255;
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, brickPixels
        );

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // 2. Floor Texture
    unsigned int floorTexture;

    glGenTextures(1, &floorTexture);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char* floorData =
        stbi_load("assets/textures/floor.png", &width, &height, &nrChannels, 0);

    if (floorData) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(
            GL_TEXTURE_2D, 0, format, width, height, 0,
            format, GL_UNSIGNED_BYTE, floorData
        );

        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(floorData);
    } else {
        unsigned char tilePixels[16 * 16 * 4];

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int index = (y * 16 + x) * 4;

                bool isGrout = (x % 8 == 0) || (y % 8 == 0);

                tilePixels[index + 0] = isGrout ? 20 : 50;
                tilePixels[index + 1] = isGrout ? 22 : 60;
                tilePixels[index + 2] = isGrout ? 25 : 70;
                tilePixels[index + 3] = 255;
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, tilePixels
        );

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // 3. Fuse Texture
    unsigned int fuseTexture;

    glGenTextures(1, &fuseTexture);
    glBindTexture(GL_TEXTURE_2D, fuseTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    {
        unsigned char fusePixels[16 * 16 * 4];

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int idx = (y * 16 + x) * 4;

                bool isRim = (x == 0 || x == 15 || y == 0 || y == 15);
                bool isCenter = (x >= 6 && x <= 9 && y >= 6 && y <= 9);

                if (isRim) {
                    fusePixels[idx + 0] = 55;
                    fusePixels[idx + 1] = 50;
                    fusePixels[idx + 2] = 35;
                } else if (isCenter) {
                    fusePixels[idx + 0] = 255;
                    fusePixels[idx + 1] = 255;
                    fusePixels[idx + 2] = 220;
                } else {
                    fusePixels[idx + 0] = 250;
                    fusePixels[idx + 1] = 205;
                    fusePixels[idx + 2] = 45;
                }

                fusePixels[idx + 3] = 255;
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, fusePixels
        );

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // 4. Closet Texture
    unsigned int closetTexture;

    glGenTextures(1, &closetTexture);
    glBindTexture(GL_TEXTURE_2D, closetTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    {
        unsigned char closetPixels[16 * 16 * 4];

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int idx = (y * 16 + x) * 4;

                bool isFrame = (x == 0 || x == 15 || y == 0 || y == 15);
                bool isSeam = (x == 7 || x == 8);
                bool isHandle = (x >= 5 && x <= 6 && y >= 7 && y <= 9);

                if (isFrame) {
                    closetPixels[idx + 0] = 25;
                    closetPixels[idx + 1] = 45;
                    closetPixels[idx + 2] = 30;
                } else if (isHandle) {
                    closetPixels[idx + 0] = 210;
                    closetPixels[idx + 1] = 180;
                    closetPixels[idx + 2] = 85;
                } else if (isSeam) {
                    closetPixels[idx + 0] = 18;
                    closetPixels[idx + 1] = 32;
                    closetPixels[idx + 2] = 22;
                } else {
                    closetPixels[idx + 0] = 45;
                    closetPixels[idx + 1] = 88;
                    closetPixels[idx + 2] = 58;
                }

                closetPixels[idx + 3] = 255;
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, closetPixels
        );

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // 5. Monster Texture: dark crimson skin
    unsigned int monsterTexture;

    glGenTextures(1, &monsterTexture);
    glBindTexture(GL_TEXTURE_2D, monsterTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    {
        unsigned char monsterPixels[16 * 16 * 4];

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int idx = (y * 16 + x) * 4;

                bool darkPatch = ((x + y) % 5 == 0);

                monsterPixels[idx + 0] = darkPatch ? 45 : 125;
                monsterPixels[idx + 1] = darkPatch ? 8 : 18;
                monsterPixels[idx + 2] = darkPatch ? 12 : 28;
                monsterPixels[idx + 3] = 255;
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, monsterPixels
        );

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // Instantiate software minimap HUD
    MinimapHUD minimapHUD(200, 200);

    // Main Render Loop
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (isPaused) {
            deltaTime = 0.0f;
        }

        processInput(window);

        // Update monster AI.
        if (!isPaused &&
            gameState != GameState::CAUGHT &&
            gameState != GameState::ESCAPED) {

            gameState = monster.update(
                deltaTime,
                cameraPos,
                interactionSystem.isPlayerHidden,
                interactionSystem.allFusesCollected(),
                houseGrid
            );
        }

        // Game state window title.
        if (gameState == GameState::CAUGHT) {
            glfwSetWindowTitle(
                window,
                "GAME OVER - The monster caught you! (ESC to pause, Q to quit)"
            );
        } else if (gameState == GameState::ESCAPED) {
            glfwSetWindowTitle(
                window,
                "YOU ESCAPED! - The house is restored!"
            );
        }

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Render 3D Scene
        glEnable(GL_DEPTH_TEST);
        sceneShader.use();

        glm::mat4 view =
            glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        float aspect = (currentHeight > 0)
            ? static_cast<float>(currentWidth) / static_cast<float>(currentHeight)
            : 16.0f / 9.0f;

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            aspect,
            0.1f,
            100.0f
        );

        int modelLoc = glGetUniformLocation(sceneShader.ID, "model");
        int viewLoc = glGetUniformLocation(sceneShader.ID, "view");
        int projLoc = glGetUniformLocation(sceneShader.ID, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Flashlight / spotlight uniforms
        glUniform3fv(
            glGetUniformLocation(sceneShader.ID, "lightPos"),
            1,
            glm::value_ptr(cameraPos)
        );

        glUniform3fv(
            glGetUniformLocation(sceneShader.ID, "lightDir"),
            1,
            glm::value_ptr(cameraFront)
        );

        float flickerFactor = flashlightOn
            ? monster.getProximityFlicker(cameraPos, currentFrame)
            : 1.0f;

        if (flashlightOn) {
            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "cutOff"),
                std::cos(glm::radians(18.0f))
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "outerCutOff"),
                std::cos(glm::radians(26.0f))
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "constant"), 1.0f
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "linear"), 0.05f
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "quadratic"), 0.015f
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "ambientStrength"), 0.20f
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "flickerFactor"),
                flickerFactor
            );
        } else {
            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "cutOff"),
                std::cos(glm::radians(85.0f))
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "outerCutOff"),
                std::cos(glm::radians(89.0f))
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "constant"), 1.0f
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "linear"), 0.01f
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "quadratic"), 0.005f
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "ambientStrength"), 0.85f
            );

            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "flickerFactor"), 1.0f
            );
            glUniform1f(
                glGetUniformLocation(sceneShader.ID, "emissiveStrength"),0.0f
            );
        }
          
        // Draw Floor
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glBindVertexArray(floorVAO);

        glm::mat4 floorModel(1.0f);

        glUniformMatrix4fv(
            modelLoc, 1, GL_FALSE, glm::value_ptr(floorModel)
        );

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw walls and closets
        glBindVertexArray(VAO);

        int mapRows = static_cast<int>(houseGrid.size());
        int mapCols = static_cast<int>(houseGrid[0].size());

        for (int z = 0; z < mapRows; ++z) {
            for (int x = 0; x < mapCols; ++x) {

                if (houseGrid[z][x] == '1') {
                    glBindTexture(GL_TEXTURE_2D, wallTexture);

                    glm::mat4 wallModel(1.0f);

                    float posX = static_cast<float>(x) - mapCols * 0.5f;
                    float posZ = static_cast<float>(z) - mapRows * 0.5f;

                    wallModel = glm::translate(
                        wallModel,
                        glm::vec3(posX, 0.0f, posZ)
                    );

                    glUniformMatrix4fv(
                        modelLoc, 1, GL_FALSE, glm::value_ptr(wallModel)
                    );

                    glDrawArrays(GL_TRIANGLES, 0, 36);
                } else if (houseGrid[z][x] == 'H') {
                    glBindTexture(GL_TEXTURE_2D, closetTexture);

                    glm::mat4 closetModel(1.0f);

                    float posX = static_cast<float>(x) - mapCols * 0.5f;
                    float posZ = static_cast<float>(z) - mapRows * 0.5f;

                    closetModel = glm::translate(
                        closetModel,
                        glm::vec3(posX, 0.2f, posZ)
                    );

                    closetModel = glm::scale(
                        closetModel,
                        glm::vec3(0.82f, 1.4f, 0.82f)
                    );

                    glUniformMatrix4fv(
                        modelLoc, 1, GL_FALSE, glm::value_ptr(closetModel)
                    );

                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
        }

        // 2. Render uncollected fuses
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, fuseTexture);

        for (const auto& fuse : interactionSystem.fuses) {

            if (fuse.collected) {
                continue;
            }

            float bobbing =
                0.65f + 0.12f * std::sin(currentFrame * 3.0f);

            float rotationAngle = currentFrame * 1.8f;

            glm::mat4 fuseModel(1.0f);

            fuseModel = glm::translate(
                fuseModel,
                glm::vec3(
                    fuse.position.x,
                    bobbing,
                    fuse.position.z
                )
            );

            fuseModel = glm::rotate(
                fuseModel,
                rotationAngle,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            fuseModel = glm::scale(
                fuseModel,
                glm::vec3(0.35f)
            );

            glUniformMatrix4fv(
                modelLoc, 1, GL_FALSE, glm::value_ptr(fuseModel)
            );

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 3. Render Monster
        
        // 3. Render Monster
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, monsterTexture);

        glm::vec3 monsterPos = monster.position;

        // Give the monster independent visibility.
        glUniform1f(
            glGetUniformLocation(sceneShader.ID, "emissiveStrength"),
            0.55f
        );

        // Body
        glm::mat4 monsterBody(1.0f);

        monsterBody = glm::translate(
            monsterBody,
            glm::vec3(monsterPos.x, 0.5f, monsterPos.z)
        );

        monsterBody = glm::scale(
            monsterBody,
            glm::vec3(0.65f, 0.9f, 0.48f)
        );

        glUniformMatrix4fv(
            modelLoc, 1, GL_FALSE, glm::value_ptr(monsterBody)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Head
        glm::mat4 monsterHead(1.0f);

        monsterHead = glm::translate(
            monsterHead,
            glm::vec3(monsterPos.x, 1.2f, monsterPos.z)
        );

        monsterHead = glm::scale(
            monsterHead,
            glm::vec3(0.48f, 0.48f, 0.48f)
        );

        glUniformMatrix4fv(
            modelLoc, 1, GL_FALSE, glm::value_ptr(monsterHead)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Arms
        for (int side : {-1, 1}) {
            glm::mat4 armModel(1.0f);

            armModel = glm::translate(
                armModel,
                glm::vec3(
                    monsterPos.x + side * 0.48f,
                    0.5f,
                    monsterPos.z
                )
            );

            armModel = glm::rotate(
                armModel,
                glm::radians(side * 12.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
            );

            armModel = glm::scale(
                armModel,
                glm::vec3(0.22f, 0.8f, 0.25f)
            );

            glUniformMatrix4fv(
                modelLoc, 1, GL_FALSE, glm::value_ptr(armModel)
            );

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Glowing eyes
        glBindTexture(GL_TEXTURE_2D, fuseTexture);

        glUniform1f(
            glGetUniformLocation(sceneShader.ID, "emissiveStrength"),
            1.2f
        );

        for (int side : {-1, 1}) {
            glm::mat4 eyeModel(1.0f);

            eyeModel = glm::translate(
                eyeModel,
                glm::vec3(
                    monsterPos.x + side * 0.12f,
                    1.25f,
                    monsterPos.z - 0.245f
                )
            );

            eyeModel = glm::scale(
                eyeModel,
                glm::vec3(0.09f, 0.07f, 0.035f)
            );

            glUniformMatrix4fv(
                modelLoc, 1, GL_FALSE, glm::value_ptr(eyeModel)
            );

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Restore normal lighting for subsequent objects.
        glUniform1f(
            glGetUniformLocation(sceneShader.ID, "emissiveStrength"),
            0.0f
        );


        // 4. Render minimap HUD and acoustic footprints
         if (showMinimapHUD) {

            minimapHUD.render(
                cameraPos.x,
                cameraPos.z,
                yaw,
                currentFrame,
                houseGrid,
                true,
                isSprinting,
                isCrouching,
                monster.position.x,
                monster.position.z,
                monster.mode == MonsterMode::CHASE
            );

            glDisable(GL_DEPTH_TEST);

            hudShader.use();

            glActiveTexture(GL_TEXTURE0);

            glBindTexture(
                GL_TEXTURE_2D,
                minimapHUD.canvas.textureID
            );

            glUniform1i(
                glGetUniformLocation(hudShader.ID, "hudTexture"),
                0
            );

            glBindVertexArray(hudVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Debug telemetry
        if (showDebugTelemetry &&
            gameState != GameState::CAUGHT &&
            gameState != GameState::ESCAPED) {

            char title[200];

            int fps = (deltaTime > 0.0f)
                ? static_cast<int>(1.0f / deltaTime)
                : 0;

            const char* playerState =
                interactionSystem.isPlayerHidden ? "HIDDEN" : "VISIBLE";

            const char* monsterState =
                monster.mode == MonsterMode::CHASE ? "CHASE" : "PATROL";

            std::snprintf(
                title,
                sizeof(title),
                "Echoes in the Dark | Fuses: %d/3 | %s | Monster: %s | "
                "Pos: (%.2f, %.2f, %.2f) | FPS: %d",
                interactionSystem.fusesCollected,
                playerState,
                monsterState,
                cameraPos.x,
                cameraPos.y,
                cameraPos.z,
                fps
            );

            glfwSetWindowTitle(window, title);
        }

        // Automatic Checkpoint 1 screenshot capture on frame 5
        static int screenshotFrame = 0;

        if (++screenshotFrame == 5) {
            SoftwareCanvas screenCanvas(currentWidth, currentHeight);

            std::vector<uint8_t> glPixels(
                currentWidth * currentHeight * 3
            );

            glReadPixels(
                0,
                0,
                currentWidth,
                currentHeight,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                glPixels.data()
            );

            for (int y = 0; y < currentHeight; ++y) {
                int srcY = currentHeight - 1 - y;

                for (int x = 0; x < currentWidth; ++x) {
                    int srcIdx = (srcY * currentWidth + x) * 3;

                    screenCanvas.setPixel(
                        x,
                        y,
                        rgba(
                            glPixels[srcIdx],
                            glPixels[srcIdx + 1],
                            glPixels[srcIdx + 2]
                        )
                    );
                }
            }

            screenCanvas.savePNG(
                "checkpoints/checkpoint1/cp1_output/cp1_gameplay_3d_hud.png"
            );
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
