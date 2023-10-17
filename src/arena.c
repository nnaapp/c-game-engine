#include <stdlib.h>
#include <string.h>
#include <memoryapi.h>
#include "../include/arena.h"

// 16gb virtual alloc max
#define VIRTUAL_ALLOC_SIZE 64000000000

// windows page size
#define PAGE_SIZE 4096

// default alignment
#define DEFAULT_ALIGN (2*(sizeof(void *)))

///////////////////////////////////////
// Arena Allocator ////////////////////
///////////////////////////////////////

// Validates if the input (memory address) is a power of 2
static bool IsPowOfTwo(uintptr_t x)
{
    return (x & (x - 1)) == 0;
}

// Ensures that the memory address passed in is aligned with the align argument
static uintptr_t AlignPtr(uintptr_t ptr, uint64_t align)
{
    uintptr_t a = align;

    // error return is -1 underflow
    if (!IsPowOfTwo(a)) return -1;

    uintptr_t mod = ptr & (a - 1);

    if (mod != 0)
    {
        ptr += a - mod;
    }

    return ptr;
}

// Allocate and set up an arena
Arena *ArenaAlloc()
{
    Arena *arena = malloc(sizeof(*arena));
    arena->arena = VirtualAlloc(NULL, VIRTUAL_ALLOC_SIZE, MEM_RESERVE, PAGE_READWRITE);
    arena->offset = 0;
    arena->pages = 0;

    return arena;
}

// Frees ALL of the memory an arena points to
void ArenaDealloc(Arena *arena)
{
    VirtualFree(arena->arena, 0, MEM_RELEASE);
    arena->offset = 0;
    arena->pages = 0;
}

// Arena allocation with an alignment argument, if the default alignment 
// is not good enough, returns pointer to the memory chunk.
// Returns NULL if there is not enough room in the allocator.
void *ArenaPush(Arena *arena, uint64_t allocSize, uint64_t align)
{
    // Get lowest unused space in arena
    uintptr_t unusedAddress = (uintptr_t)arena->arena + (uintptr_t)arena->offset;
    // Align that position, and get offset from start of arena
    uintptr_t arenaOffset = AlignPtr(unusedAddress, align) - (uintptr_t)arena->arena;

    // Get ptr and change offset to reflect space being allocated
    void *ptr = &arena->arena[arenaOffset];
    int32_t nextPageDistance = (arenaOffset + allocSize) - (arena->pages * PAGE_SIZE);
    if (nextPageDistance > 0)
    {
        int32_t newPages = (nextPageDistance / PAGE_SIZE) + 1;
        arena->pages += newPages;
        VirtualAlloc(ptr, newPages * PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE);
    }
    arena->offset = arenaOffset + allocSize;

    return ptr;
}

void *ArenaPushZero(Arena *arena, uint64_t allocSize, uint64_t align)
{
    void *ptr = ArenaPush(arena, allocSize, align);
    // Set memory to 0
    memset(ptr, 0, allocSize);
    return ptr;
}

void ArenaPop(Arena *arena, uint64_t popSize)
{
    arena->offset -= popSize;
}

void ArenaClear(Arena *arena)
{
    arena->offset = 0;
}

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////

