#ifndef SOFTWARE_CANVAS_HPP
#define SOFTWARE_CANVAS_HPP

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <glad/glad.h>

// Color pack helper: 0xRRGGBBAA
inline constexpr uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (uint32_t(r) << 24) | (uint32_t(g) << 16) | (uint32_t(b) << 8) | uint32_t(a);
}

// CPU-side RGBA pixel buffer. All pure C++ algorithm outputs write here.
// uploadToGPU() pushes the buffer to an OpenGL texture for runtime HUD rendering.
// savePNG() writes the buffer directly to a PNG file with zero third-party libraries.
class SoftwareCanvas {
public:
    int width;
    int height;
    std::vector<uint32_t> pixels; // 0xRRGGBBAA, row-major
    unsigned int textureID = 0;

    SoftwareCanvas(int w, int h, uint32_t clearColor = 0x000000FF)
        : width(w), height(h), pixels(w * h, clearColor) {}

    void clear(uint32_t color = 0x000000FF) {
        std::fill(pixels.begin(), pixels.end(), color);
    }

    void setPixel(int x, int y, uint32_t color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y * width + x] = color;
        }
    }

    uint32_t getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return pixels[y * width + x];
        }
        return 0;
    }

    // Upload pixel buffer to an OpenGL texture for HUD rendering
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                    GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels.data());
    }

    // Zero-dependency pure C++ PNG export (uncompressed RFC 1951 deflate blocks)
    bool savePNG(const std::string& path) const {
        std::ofstream f(path, std::ios::binary);
        if (!f) return false;

        auto crc32 = [](const std::vector<uint8_t>& d) -> uint32_t {
            static std::array<uint32_t, 256> t = [] {
                std::array<uint32_t, 256> a{};
                for (uint32_t n = 0; n < 256; ++n) {
                    uint32_t c = n;
                    for (int k = 0; k < 8; ++k) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
                    a[n] = c;
                }
                return a;
            }();
            uint32_t c = 0xFFFFFFFFu;
            for (uint8_t b : d) c = t[(c ^ b) & 0xFF] ^ (c >> 8);
            return c ^ 0xFFFFFFFFu;
        };

        auto putU32 = [](std::vector<uint8_t>& v, uint32_t x) {
            for (int s = 24; s >= 0; s -= 8) v.push_back(uint8_t(x >> s));
        };

        auto chunk = [&](const char* type, const std::vector<uint8_t>& data) {
            std::vector<uint8_t> td(type, type + 4), out;
            td.insert(td.end(), data.begin(), data.end());
            putU32(out, uint32_t(data.size()));
            out.insert(out.end(), td.begin(), td.end());
            putU32(out, crc32(td));
            f.write(reinterpret_cast<const char*>(out.data()), std::streamsize(out.size()));
        };

        const uint8_t sig[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
        f.write(reinterpret_cast<const char*>(sig), 8);

        std::vector<uint8_t> ihdr;
        putU32(ihdr, uint32_t(width));
        putU32(ihdr, uint32_t(height));
        ihdr.insert(ihdr.end(), {8, 2, 0, 0, 0}); // 8-bit RGB
        chunk("IHDR", ihdr);

        std::vector<uint8_t> raw;
        raw.reserve(size_t(height) * (size_t(width) * 3 + 1));
        for (int y = 0; y < height; ++y) {
            raw.push_back(0); // filter type: None
            for (int x = 0; x < width; ++x) {
                uint32_t p = pixels[y * width + x];
                raw.push_back(static_cast<uint8_t>((p >> 24) & 0xFF)); // R
                raw.push_back(static_cast<uint8_t>((p >> 16) & 0xFF)); // G
                raw.push_back(static_cast<uint8_t>((p >> 8)  & 0xFF)); // B
            }
        }

        std::vector<uint8_t> z = {0x78, 0x01};
        for (size_t pos = 0; pos < raw.size();) {
            size_t len = std::min<size_t>(65535, raw.size() - pos);
            z.push_back(pos + len == raw.size() ? 1 : 0);
            z.push_back(uint8_t(len & 0xFF));
            z.push_back(uint8_t(len >> 8));
            z.push_back(uint8_t(~len & 0xFF));
            z.push_back(uint8_t((~len >> 8) & 0xFF));
            z.insert(z.end(), raw.begin() + pos, raw.begin() + pos + len);
            pos += len;
        }

        uint32_t a = 1, b = 0;
        for (uint8_t v : raw) { a = (a + v) % 65521; b = (b + a) % 65521; }
        putU32(z, (b << 16) | a);
        chunk("IDAT", z);
        chunk("IEND", {});

        return bool(f);
    }
};

#endif // SOFTWARE_CANVAS_HPP
