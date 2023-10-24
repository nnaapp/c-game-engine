#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdint.h>
#include "../include/raylib.h"

typedef struct Vector2Int
{
    int32_t x;
    int32_t y;
} Vector2Int;

typedef struct Line
{
    Vector2 start;
    Vector2 end;
    Color color;
    float thickness;
} Line;

bool Vector2Compare(Vector2 v1, Vector2 v2)
{
    if (v1.x != v2.x) return false;
    if (v1.y != v2.y) return false;

    return true;
}

bool Vector2InRec(Vector2 v, Rectangle rect)
{
    return (v.x > rect.x) && (v.x < (rect.x + rect.width)) && (v.y > rect.y) && (v.y < (rect.y + rect.height));
}

#endif
