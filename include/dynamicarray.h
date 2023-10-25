#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "../include/arena.h"
#include "../include/raylib.h"
#include "../include/windows_utils.h"
#include <stdint.h>

typedef struct DynamicTiles
{
    
    Arena *arena;
    Color *values;
    int32_t len;
    int32_t capacity;
} DynamicTiles;

#define InitDynamicArray(dynamicArray, type) {\
    dynamicArray.arena = ArenaAlloc();\
    dynamicArray.values = (type *)dynamicArray.arena->arena;\
    dynamicArray.len = 0;\
    dynamicArray.capacity = 0;\
}
#define PushArrayDynamic(dynamicArray, type, value) {\
    PushStruct(dynamicArray.arena, type);\
    dynamicArray.capacity++;\
    dynamicArray.values[dynamicArray.len] = value;\
    dynamicArray.len++;\
}
#define PopArrayDynamic(dynamicArray, count) {\
    ArenaPop(dynamicArray.arena, sizeof(*dynamicArray.values) * count);\
    dynamicArray.capacity -= count;\
    if (dynamicArray.len >= dynamicArray.capacity) dynamicArray.len = dynamicArray.capacity;\
}
#define AppendArrayDynamic(dynamicArray, value) dynamicArray.values[dynamicArray.len] = value, dynamicArray.len++;
#define DynamicResize(dynamicArray, type, size) {\
    if (dynamicArray.capacity > size)\
    {\
        ArenaPop(dynamicArray.arena, (dynamicArray.capacity - size) * sizeof(type));\
        if (dynamicArray.len >= size) dynamicArray.len = size;\
    }\
    else\
    {\
        PushArray(dynamicArray.arena, type, size - dynamicArray.capacity);\
    }\
    dynamicArray.capacity = size;\
}

#endif
