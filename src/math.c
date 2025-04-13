#include "math.h"

void vec3_cross(const float* a, const float* b, float* result) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

void vec3_normalize(float* v) {
    float length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (length != 0.0f) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}
