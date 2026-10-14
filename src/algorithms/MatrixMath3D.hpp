#ifndef MATRIX_MATH_3D_HPP
#define MATRIX_MATH_3D_HPP

#include <cmath>

// Pure C++ 3D transformation and projection mathematics.
// Implements homogeneous 4x4 matrix transforms, perspective projection,
// and camera view matrix generation without external libraries.

namespace cp1 {

struct Vec3 { float x, y, z; };
struct Vec4 { float x, y, z, w; };

struct Mat4 {
    float m[4][4]{};
    static Mat4 identity() {
        Mat4 r;
        for (int i = 0; i < 4; ++i) r.m[i][i] = 1.0f;
        return r;
    }
};

inline Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 r;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
    return r;
}

inline Vec4 operator*(const Mat4& a, Vec4 v) {
    return {
        a.m[0][0]*v.x + a.m[0][1]*v.y + a.m[0][2]*v.z + a.m[0][3]*v.w,
        a.m[1][0]*v.x + a.m[1][1]*v.y + a.m[1][2]*v.z + a.m[1][3]*v.w,
        a.m[2][0]*v.x + a.m[2][1]*v.y + a.m[2][2]*v.z + a.m[2][3]*v.w,
        a.m[3][0]*v.x + a.m[3][1]*v.y + a.m[3][2]*v.z + a.m[3][3]*v.w
    };
}

inline Mat4 translate(float x, float y, float z) {
    Mat4 r = Mat4::identity();
    r.m[0][3] = x; r.m[1][3] = y; r.m[2][3] = z;
    return r;
}

inline Mat4 scale(float x, float y, float z) {
    Mat4 r = Mat4::identity();
    r.m[0][0] = x; r.m[1][1] = y; r.m[2][2] = z;
    return r;
}

inline Mat4 rotateY(float a) {
    Mat4 r = Mat4::identity();
    float c = std::cos(a), s = std::sin(a);
    r.m[0][0] = c; r.m[0][2] = s;
    r.m[2][0] = -s; r.m[2][2] = c;
    return r;
}

inline float dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

inline Vec3 norm(Vec3 v) {
    float l = std::sqrt(dot(v, v));
    return (l > 1e-6f) ? Vec3{v.x / l, v.y / l, v.z / l} : Vec3{0, 0, 0};
}

inline Mat4 lookAt(Vec3 eye, Vec3 ctr, Vec3 up) {
    Vec3 f = norm({ctr.x - eye.x, ctr.y - eye.y, ctr.z - eye.z});
    Vec3 s = norm(cross(f, up));
    Vec3 u = cross(s, f);
    Mat4 r = Mat4::identity();
    r.m[0][0] = s.x;  r.m[0][1] = s.y;  r.m[0][2] = s.z;  r.m[0][3] = -dot(s, eye);
    r.m[1][0] = u.x;  r.m[1][1] = u.y;  r.m[1][2] = u.z;  r.m[1][3] = -dot(u, eye);
    r.m[2][0] = -f.x; r.m[2][1] = -f.y; r.m[2][2] = -f.z; r.m[2][3] =  dot(f, eye);
    return r;
}

inline Mat4 perspective(float fovY, float aspect, float n, float f) {
    float t = 1.0f / std::tan(fovY / 2.0f);
    Mat4 r;
    r.m[0][0] = t / aspect;
    r.m[1][1] = t;
    r.m[2][2] = (f + n) / (n - f);
    r.m[2][3] = (2.0f * f * n) / (n - f);
    r.m[3][2] = -1.0f;
    return r;
}

} // namespace cp1

#endif // MATRIX_MATH_3D_HPP
