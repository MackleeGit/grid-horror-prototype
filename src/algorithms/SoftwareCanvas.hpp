#ifndef SOFTWARE_CANVAS_HPP
#define SOFTWARE_CANVAS_HPP

#include <vector>
#include <cstdint>
#include <glad/glad.h>

/*
================================================================================
SOFTWARE GRAPHICS CANVAS (PURE C++ PIXEL BUFFER)
================================================================================
(a) What concept does this introduce?
    A CPU-side software rasterization framebuffer (2D array of RGBA pixels).
    This decouples all underlying 2D rendering logic from hardware-accelerated
    graphics APIs (OpenGL).

(b) Why is this the correct way to do it at this stage?
    To satisfy the requirement of writing pure C++ computer graphics algorithms,
    our algorithm implementations must directly manipulate raw pixel memory
    (color buffers) rather than issuing GL hardware draw calls.

(c) What would break if skipped?
    Without a CPU pixel buffer, algorithm outputs (Bresenham lines, Midpoint 
    circles, Scanline fills) could not be rendered or visually verified without
    relying on GPU graphics pipeline calls.
================================================================================
*/

class SoftwareCanvas {
public:
    int width;
    int height;
    std::vector<uint32_t> pixels; // Raw RGBA 32-bit pixel buffer
    unsigned int textureID = 0;

    SoftwareCanvas(int w, int h) : width(w), height(h), pixels(w * h, 0x000000FF) {}

    // Clear buffer with a solid RGBA color
    void clear(uint32_t color = 0x000000FF) {
        std::fill(pixels.begin(), pixels.end(), color);
    }

    // Set pixel safely within canvas bounds
    void setPixel(int x, int y, uint32_t color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y * width + x] = color;
        }
    }

    // Get pixel color at (x, y)
    uint32_t getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return pixels[y * width + x];
        }
        return 0;
    }

    // Upload CPU pixel array to an OpenGL 2D texture for display as a HUD
    void uploadToGPU() {
        if (textureID == 0) {
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }
};

#endif // SOFTWARE_CANVAS_HPP
