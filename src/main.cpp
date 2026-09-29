#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// -----------------------------------------------------------------------------
// [JUSTIFICATION: Screen Dimensions & Viewport Synchronization]
// (a) What concept does this introduce?
//     Window framebuffer dimension tracking and dynamic viewport mapping.
// (b) Why is this the correct way to do it at this stage of the pipeline?
//     OpenGL maps normalized device coordinates (NDC [-1, 1]) to screen space
//     pixels via glViewport. Listening to resize events ensures coordinate integrity.
// (c) What would break if omitted?
//     Resizing or maximizing the window would freeze the rasterizer at the initial
//     resolution, causing stretching, clipping, or incorrect aspect ratios.
// -----------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// -----------------------------------------------------------------------------
// [JUSTIFICATION: FPS Camera Representation via Vectors and Euler Angles]
// (a) What concept does this introduce?
//     A 3D First-Person View model using spherical coordinates (Yaw, Pitch)
//     translated into Cartesian unit vectors via trigonometric projection.
// (b) Why is this the correct way to do it at this stage of the pipeline?
//     OpenGL has no camera object; a camera is mathematically a coordinate
//     transformation matrix. Decoupling translation (cameraPos) from orientation
//     (cameraFront computed from yaw/pitch) allows independent rotation and movement.
// (c) What would break if omitted?
//     Without Euler angles and directional vectors, the camera cannot look around
//     smoothly in 3D space. Without pitch clamping, looking directly up or down
//     causes the cross-product with world Up to invert, producing a disorienting screen flip.
// -----------------------------------------------------------------------------
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f; // -90.0f points the camera along the negative Z-axis by default
float pitch =  0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// -----------------------------------------------------------------------------
// [JUSTIFICATION: Delta Time & Framerate Independence]
// (a) What concept does this introduce?
//     Time-step delta measurement between rendering frames using the system clock.
// (b) Why is this the correct way to do it at this stage of the pipeline?
//     Graphics hardware and monitor refresh rates vary wildly across systems.
//     Multiplying movement speed by deltaTime guarantees identical movement
//     velocities regardless of whether the system runs at 30, 60, or 240 FPS.
// (c) What would break if omitted?
//     Player velocity would be locked directly to the hardware frame rate. The
//     player would sprint uncontrollably on high-refresh-rate displays and crawl on slower ones.
// -----------------------------------------------------------------------------
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// -----------------------------------------------------------------------------
// [JUSTIFICATION: Mouse Cursor Callback & Spherical-to-Cartesian Math]
// (a) What concept does this introduce?
//     Relative mouse offset accumulation and Euler-angle spherical-to-Cartesian conversion.
// (b) Why is this the correct way to do it at this stage of the pipeline?
//     Raw absolute mouse coordinates are meaningless when the cursor is trapped;
//     calculating frame-to-frame delta (xoffset, yoffset) scaled by sensitivity
//     produces natural, fluid head rotation.
// (c) What would break if omitted?
//     The first mouse movement would calculate an offset against (0,0), causing a
//     jarring camera snap. Without pitch clamping [-89, 89], the view would flip inverted.
// -----------------------------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since Y-coordinates range bottom-to-top
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    // Constrain pitch to avoid screen flipping (gimbal lock with world Up vector)
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Convert spherical Euler angles to Cartesian direction vector
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// -----------------------------------------------------------------------------
// [JUSTIFICATION: Debug Telemetry & F3 Key Toggle]
// (a) What concept does this introduce?
//     Real-time application state inspection and debounced key toggle logic.
// (b) Why is this the correct way to do it at this stage of the pipeline?
//     Prior to rendering 3D geometry in Phase 2, visual verification of camera
//     coordinates, orientation angles, and frame delta is essential.
//     Updating the OS window title bar provides zero-overhead telemetry without
//     requiring a full 2D text-rendering pipeline prematurely.
// (c) What would break if omitted?
//     The developer cannot verify that camera vectors and keyboard/mouse
//     inputs are calculating properly in the Phase 1 black void.
// -----------------------------------------------------------------------------
bool showDebugTelemetry = false;
bool f3PressedLastFrame  = false;

// -----------------------------------------------------------------------------
// [JUSTIFICATION: Keyboard Input Processing & Vector Strafe Movement]
// (a) What concept does this introduce?
//     Local-space translation along camera axes using vector cross products.
// (b) Why is this the correct way to do it at this stage of the pipeline?
//     Pressing 'W' and 'S' moves along the forward view vector (cameraFront).
//     Pressing 'A' and 'D' moves perpendicular to the camera using the cross
//     product normalize(cross(cameraFront, cameraUp)), enabling intuitive strafing.
// (c) What would break if omitted?
//     The player could only navigate along static global coordinate axes (e.g. world X/Z),
//     meaning turning the mouse would not change what direction 'Forward' moves toward.
// -----------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle debug telemetry with F3 (debounced)
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
        if (!f3PressedLastFrame) {
            showDebugTelemetry = !showDebugTelemetry;
            f3PressedLastFrame = true;
            if (!showDebugTelemetry) {
                glfwSetWindowTitle(window, "Echoes in the Dark");
            }
        }
    } else if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_RELEASE) {
        f3PressedLastFrame = false;
    }

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

int main() {
    // -------------------------------------------------------------------------
    // [JUSTIFICATION: GLFW Initialization & Core Profile Hints]
    // (a) What concept does this introduce?
    //     OS windowing abstraction and explicit OpenGL version targeting.
    // (b) Why is this the correct way to do it at this stage of the pipeline?
    //     Modern OpenGL (3.3+) deprecates fixed-function pipeline features (e.g.
    //     glBegin/glEnd). Requesting CORE_PROFILE ensures legacy functions are
    //     disabled so we only learn modern, shader-driven programmable pipeline practices.
    // (c) What would break if omitted?
    //     Drivers might fall back to a legacy compatibility context, allowing bad
    //     habits or failing to expose modern GLSL features correctly.
    // -------------------------------------------------------------------------
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Echoes in the Dark", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Capture and hide the mouse cursor for first-person camera control
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // -------------------------------------------------------------------------
    // [JUSTIFICATION: GLAD Runtime Function Loader]
    // (a) What concept does this introduce?
    //     Dynamic linking of GPU driver function pointers at runtime.
    // (b) Why is this the correct way to do it at this stage of the pipeline?
    //     OpenGL is an API specification implemented by GPU driver vendors.
    //     Function pointers must be queried dynamically at runtime per driver;
    //     GLAD abstracts this mechanism cleanly.
    // (c) What would break if omitted?
    //     Any call to core OpenGL functions beyond OpenGL 1.1 (like glGenBuffers,
    //     glUseProgram) would dereference null pointers and crash the application.
    // -------------------------------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // -------------------------------------------------------------------------
    // [JUSTIFICATION: The Main Render Loop & View Transformation Calculation]
    // (a) What concept does this introduce?
    //     The continuous frame execution cycle and real-time View Matrix calculation.
    // (b) Why is this the correct way to do it at this stage of the pipeline?
    //     Each frame queries input, updates delta time, clears buffers, computes
    //     the view matrix with glm::lookAt, and prepares for geometry rendering.
    // (c) What would break if omitted?
    //     Without updating the View matrix per frame, camera movement would not
    //     reflect on screen once 3D world geometry is added in Phase 2.
    // -------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        // Calculate per-frame delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process keyboard input
        processInput(window);

        // Clear screen to black void (Phase 1 deliverable)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Compute the View Matrix from the current camera state
        // (In Phase 2 & 3, this view matrix will be passed to GLSL shaders)
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Update F3 debug telemetry in window title if toggled on
        if (showDebugTelemetry) {
            char title[128];
            int fps = (deltaTime > 0.0f) ? static_cast<int>(1.0f / deltaTime) : 0;
            snprintf(title, sizeof(title),
                     "Echoes in the Dark [DEBUG F3] | Pos: (%.2f, %.2f, %.2f) | Yaw: %.1f Pitch: %.1f | FPS: %d",
                     cameraPos.x, cameraPos.y, cameraPos.z, yaw, pitch, fps);
            glfwSetWindowTitle(window, title);
        }

        // Swap buffers and poll OS events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup resources before termination
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
