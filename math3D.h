#ifndef MATH3D_H
#define MATH3D_H

#include <QRgb>

typedef struct alignas(16) vertex {
    float x;
    float y;
    float z;

    struct vertex& operator+=(const vertex& b);
    struct vertex& operator+=(const float& b);
    struct vertex& operator-=(const vertex& b);
    struct vertex& operator-=(const float& b);
    struct vertex& operator*=(const float& b);
    struct vertex& operator*=(const vertex& b);
    struct vertex  operator-();
} vertex;

typedef struct alignas(128) obj {
    int index_start;
    int index_end;
    int vertex_start;
    int vertex_end;
    vertex rot;
    vertex pos;
    vertex scale;
    QRgb color;
} obj;

typedef struct alignas(4) color {
    unsigned char r;
    unsigned char g;
    unsigned char b;

    bool operator==(const color & other) {
        return r = other.r && g == other.g && b == other.b;
    }

} color;

#endif // MATH3D_H
