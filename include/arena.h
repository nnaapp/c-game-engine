#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <stdlib.h>

///////////////////////////////////////
// Arena Allocator ////////////////////
///////////////////////////////////////

// Arena allocator, store multiple allocations in one place
// to bundle frees together
typedef struct Arena
{
    unsigned char *arena;
    uint64_t offset;
    uint32_t pages;
} Arena;

// Allocate an arena
Arena *ArenaAlloc();
// Deallocate an arena
void ArenaDealloc(Arena *arena);

// Push some amount of bytes onto the arena
void *ArenaPush(Arena *arena, uint64_t allocSize, uint64_t align);
// Push some amount of zero bytes onto the arena
void *ArenaPushZero(Arena *arena, uint64_t allocSize, uint64_t align);

// Remove some amount of bytes from the top of the arena
void ArenaPop(Arena *arena, uint64_t popSize);

// Empty out arena, set offset to 0
void ArenaClear(Arena *arena);

// Macros for ease of read/writing
#define PushDefaultAlign(arena, count) ArenaPush(arena, size, DEFAULT_ALIGN)
#define PushArray(arena, type, count) ArenaPush(arena, sizeof(type) * count, alignof(type))
#define PushArrayZero(arena, type, count) ArenaPushZero(arena, sizeof(type) * count, alignof(type))
#define PushStruct(arena, type) PushArray(arena, type, 1)
#define PushStructZero(arena, type) PushArrayZero(arena, type, 1)

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////

#endif
