#ifndef MATH_H
#define MATH_H

#include "defines.h"

typedef struct
{
    float x;
    float y;
} math_vec2f;

typedef struct
{
    float x;
    float y;
    float z;
} math_vec3f;

void vec3_cross(const float* a, const float* b, float* result);
void vec3_normalize(float* v);

#endif
