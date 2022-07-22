#include "Vector2D.h"

namespace Emulator {

std::ostream& operator<<(std::ostream& out, const Vector2D& v) {
    out << v.x << " " << v.y;
    return out;
}

std::istream& operator>>(std::istream& in, Vector2D& v) {
    in >> v.x >> v.y;
    return in;
}

Vector2D RandomUniformVector() {
    double fx = (double)rand() / RAND_MAX;
    double fy = (double)rand() / RAND_MAX;
    return {2 * fx - 1, 2 * fy - 1};
}

int sign(double x) {
    return x > 0;
}

bool SegmentIntersectsCircle(Vector2D p1, Vector2D p2, Vector2D center, double radius) {
    auto c1 = center - p1;
    auto c2 = center - p2;
    auto t = p2 - p1;
    if (abs2(c1) < radius * radius || abs2(c2) < radius * radius) {
        return true;
    }

    if (sign(c1 * t) == sign(c2 * t)) {
        return false;
    }

    return (fabs(t % c1) / abs(t) < radius);
}

Vector2D CropDirection(Vector2D direction, Vector2D base, double angle) {
    direction = norm(direction);
    base = norm(base);

    if (direction * base > cos(angle)) {
        return direction;
    }

    auto baseT = norm(direction - base * (direction * base));

    return base * cos(angle) + baseT * sin(angle);
}

}
