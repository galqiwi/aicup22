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

inline double abs2(Vector2D a) {
    return a.x * a.x + a.y * a.y;
}

inline double abs(Vector2D a) {
    return sqrt(abs2(a));
}

inline Vector2D norm(Vector2D a) {
    return a / abs(a);
}

std::ostream& operator<<(std::ostream& out, const Vector2D& v);

std::istream& operator>>(std::istream& in, Vector2D& v);

Vector2D RandomUniformVector();

}
