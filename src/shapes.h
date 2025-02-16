#ifndef SHAPES_H
#define SHAPES_H

typedef struct Rectangle
{
    float x;
    float y;
    float width;
    float height;
} Rectangle;

typedef struct vec2
{
    float x;
    float y;
} vec2;

typedef struct vec3
{
    float x;
    float y;
    float z;
} vec3;

typedef struct vec4
{
    float x;
    float y;
    float z;
    float w;
} vec4;

typedef struct color
{
    float x;
    float y;
    float z;
    float w;
} color;

void DrawRectangle(Rectangle rec, vec2 origin, float rotation, color color);

#endif