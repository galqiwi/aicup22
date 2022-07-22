#pragma once

#include <cmath>
#include "model/Vec2.hpp"

namespace Emulator {

struct Vector2D {
    inline static Vector2D FromApi(model::Vec2 v) {
        return {v.x, v.y};
    }

    inline model::Vec2 ToApi() const {
        return {x, y};
    }

    double x, y;
};

inline Vector2D operator+(Vector2D a, Vector2D b) {
    return {a.x + b.x, a.y + b.y};
}

inline Vector2D operator-(Vector2D a, Vector2D b) {
    return {a.x - b.x, a.y - b.y};
}

inline Vector2D operator*(Vector2D a, double x) {
    return {a.x * x, a.y * x};;
}

inline Vector2D operator/(Vector2D a, double x) {
    return {a.x / x, a.y / x};
}

inline double operator*(Vector2D a, Vector2D b) {
    return a.x * b.x + a.y * b.y;
}

inline double operator%(Vector2D a, Vector2D b) {
    return a.x * b.y - a.y * b.x;
}

inline double abs2(Vector2D a) {
    return a.x * a.x + a.y * a.y;
}

inline double abs(Vector2D a) {
    return sqrt(abs2(a));
}

inline Vector2D norm(Vector2D a) {
    return a / abs(a);
}

inline Vector2D rot90(Vector2D a) {
    return {a.y, -a.x};
}

std::ostream& operator<<(std::ostream& out, const Vector2D& v);

std::istream& operator>>(std::istream& in, Vector2D& v);

Vector2D RandomUniformVector();

bool SegmentIntersectsCircle(Vector2D p1, Vector2D p2, Vector2D center, double radius);

Vector2D CropDirection(Vector2D direction, Vector2D base, double angle);

}
