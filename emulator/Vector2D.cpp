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

}
