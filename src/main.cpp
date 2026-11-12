
#include "../checkpoints/checkpoint1/cp1Geometry.h"
#include "../external/stb_image.h"
#include "Shader.hpp"
#include "algorithms/MinimapHUD.hpp"
#include "gameplay/InteractionSystem.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

// Screen and viewport
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Master level house layout
cp1::Grid houseGrid = cp1::defaultHouse();

// Gameplay interaction system
InteractionSystem interactionSystem;

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

// Framebuffer Callback
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Mouse Callback
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
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

    if (pitch > 89.0f)
        pitch = 89.0f;

    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(direction);
}

// Input Processing
void processInput(GLFWwindow *window) {

    // Exit game
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // F3: Toggle debug telemetry
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
        if (!f3PressedLastFrame) {
            showDebugTelemetry = !showDebugTelemetry;
            f3PressedLastFrame = true;

            if (!showDebugTelemetry) {
                glfwSetWindowTitle(window, "Echoes in the Dark");
            }
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

    // Hiding prevents movement and therefore prevents movement footprints.
    isMoving = movementInput && !interactionSystem.isPlayerHidden;

    // Hiding locks player movement.
    if (!interactionSystem.isPlayerHidden) {

        float movementSpeed = 2.5f;

        if (isSprinting) {
            movementSpeed = 4.5f;
        } else if (isCrouching) {
            movementSpeed = 1.2f;
        }

        float cameraSpeed = movementSpeed * deltaTime;

        // Keep movement on the horizontal XZ plane.
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

        // Normalize diagonal movement.
        if (glm::length(movement) > 0.0f) {
            movement = glm::normalize(movement);
            cameraPos += movement * cameraSpeed;
        }
    }

    // Lock eye height.
    cameraPos.y = 0.0f;
}

int main() {

    // Register the three fuse objectives from the house grid.
    interactionSystem.initialize(houseGrid);

    // CP1 pure C++ geometry demos
    cp1::runGeometryDemo();

    // Export static minimap HUD frame showing all five algorithms.
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
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "Echoes in the Dark",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD Runtime Loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Depth Testing
    glEnable(GL_DEPTH_TEST);

    Shader sceneShader("shaders/scene.vert", "shaders/scene.frag");
    Shader hudShader("shaders/hud.vert", "shaders/hud.frag");

    // Wall cube geometry — 36 vertices, 6 faces
    float vertices[] = {
        // positions          // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f
    };

    unsigned int VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void *)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // Floor Geometry (EBO)
    float floorVertices[] = {
        -8.5f, -0.5f, -6.5f, 0.0f, 12.0f,
        8.5f, -0.5f, -6.5f, 16.0f, 12.0f,
        8.5f, -0.5f, 6.5f, 16.0f, 0.0f,
        -8.5f, -0.5f, 6.5f, 0.0f, 0.0f
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
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void *)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // HUD quad — top-right corner, screen space NDC coords
    float hudQuadVertices[] = {
        // positions   // texture coords (V inverted for OpenGL texture alignment)
        0.45f, 0.95f, 0.0f,  0.0f,  0.95f, 0.95f,
        1.0f,  0.0f,  0.95f, 0.45f, 1.0f,  1.0f,

        0.45f, 0.95f, 0.0f,  0.0f,  0.95f, 0.45f,
        1.0f,  1.0f,  0.45f, 0.45f, 0.0f,  1.0f
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
        0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
        (void *)(2 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // Textures — try to load from file, fall back to a procedural pattern
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;

    // 1. Wall Texture
    unsigned int wallTexture;

    glGenTextures(1, &wallTexture);
    glBindTexture(GL_TEXTURE_2D, wallTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char *wallData =
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
        // Procedural Dark Brick Wall Texture (16x16 pixels)
        unsigned char brickPixels[16 * 16 * 4];

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int index = (y * 16 + x) * 4;

                bool isMortar =
                    (y % 4 == 0) ||
                    ((y / 4) % 2 == 0
                        ? (x % 8 == 0)
                        : ((x + 4) % 8 == 0));

                if (isMortar) {
                    brickPixels[index + 0] = 40;
                    brickPixels[index + 1] = 42;
                    brickPixels[index + 2] = 45;
                    brickPixels[index + 3] = 255;
                } else {
                    brickPixels[index + 0] = 95;
                    brickPixels[index + 1] = 55;
                    brickPixels[index + 2] = 50;
                    brickPixels[index + 3] = 255;
                }
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char *floorData =
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
        // Procedural Dark Stone Tile Floor Texture (16x16 pixels)
        unsigned char tilePixels[16 * 16 * 4];

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int index = (y * 16 + x) * 4;

                bool isGrout = (x % 8 == 0) || (y % 8 == 0);

                if (isGrout) {
                    tilePixels[index + 0] = 20;
                    tilePixels[index + 1] = 22;
                    tilePixels[index + 2] = 25;
                    tilePixels[index + 3] = 255;
                } else {
                    tilePixels[index + 0] = 50;
                    tilePixels[index + 1] = 60;
                    tilePixels[index + 2] = 70;
                    tilePixels[index + 3] = 255;
                }
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, tilePixels
        );

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // Instantiate Software Minimap HUD
    MinimapHUD minimapHUD(200, 200);

    // Main Render Loop
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Render 3D Scene Geometry
        glEnable(GL_DEPTH_TEST);
        sceneShader.use();

        glm::mat4 view =
            glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glm::mat4 projection =
            glm::perspective(
                glm::radians(45.0f),
                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                0.1f,
                100.0f
            );

        int modelLoc = glGetUniformLocation(sceneShader.ID, "model");
        int viewLoc = glGetUniformLocation(sceneShader.ID, "view");
        int projLoc = glGetUniformLocation(sceneShader.ID, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Draw Floor with Floor Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glBindVertexArray(floorVAO);

        glm::mat4 floorModel = glm::mat4(1.0f);

        glUniformMatrix4fv(
            modelLoc, 1, GL_FALSE, glm::value_ptr(floorModel)
        );

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw Walls with Wall Texture
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        glBindVertexArray(VAO);

        int mapRows = static_cast<int>(houseGrid.size());
        int mapCols = static_cast<int>(houseGrid[0].size());

        for (int z = 0; z < mapRows; z++) {
            for (int x = 0; x < mapCols; x++) {

                if (houseGrid[z][x] == '1') {
                    glm::mat4 wallModel = glm::mat4(1.0f);

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
                }
            }
        }

        // 2. Render floating, rotating fuse cubes
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, wallTexture);

        for (const auto& fuse : interactionSystem.fuses) {

            // Do not render collected fuses.
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
                modelLoc,
                1,
                GL_FALSE,
                glm::value_ptr(fuseModel)
            );

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 3. Render minimap HUD and acoustic footprints
        if (showMinimapHUD) {

            minimapHUD.render(
                cameraPos.x,
                cameraPos.z,
                yaw,
                currentFrame,
                houseGrid,
                true,
                isSprinting,
                isCrouching
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
        if (showDebugTelemetry) {
            char title[160];

            int fps =
                (deltaTime > 0.0f)
                    ? static_cast<int>(1.0f / deltaTime)
                    : 0;

            const char* playerState =
                interactionSystem.isPlayerHidden ? "HIDDEN" : "VISIBLE";

            std::snprintf(
                title,
                sizeof(title),
                "Echoes in the Dark | Fuses: %d/3 | %s | "
                "Pos: (%.2f, %.2f, %.2f) | FPS: %d",
                interactionSystem.fusesCollected,
                playerState,
                cameraPos.x,
                cameraPos.y,
                cameraPos.z,
                fps
            );

            glfwSetWindowTitle(window, title);
        }

        // Automatic Checkpoint 1 evidence screenshot capture on frame 5
        static int screenshotFrame = 0;

        if (++screenshotFrame == 5) {
            SoftwareCanvas screenCanvas(SCR_WIDTH, SCR_HEIGHT);

            std::vector<uint8_t> glPixels(SCR_WIDTH * SCR_HEIGHT * 3);

            glReadPixels(
                0,
                0,
                SCR_WIDTH,
                SCR_HEIGHT,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                glPixels.data()
            );

            for (int y = 0; y < (int)SCR_HEIGHT; ++y) {
                int srcY = (int)SCR_HEIGHT - 1 - y;

                for (int x = 0; x < (int)SCR_WIDTH; ++x) {
                    int srcIdx = (srcY * SCR_WIDTH + x) * 3;

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
