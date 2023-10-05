#ifndef SERIAL_EDITOR_H
#define SERIAL_EDITOR_H

#include <stdint.h>
#include <stdbool.h>
#include "../include/arena.h"

#define MAX_STR_SIZE 256
#define MAX_COMPONENTS 32

///////////////////////////////////////
// Serial Components //////////////////
///////////////////////////////////////


typedef struct Component
{
    char *name;
    char **types;
    uint32_t maxMembers;
    uint32_t numMembers;
} Component;

// Initialize a component and allocate memory for the strings it contains
// returns false if allocation failed
bool ComponentInit(Arena *arena, Component *component, const char *name, uint32_t maxMembers);

// Add a type and value to the types and vals arrays in the given component
bool ComponentAddTypePtr(Component *component, const char *type);

// Remove the member at the given index in both the types and vals arrays
bool ComponentRemoveTypePtr(Component *component, uint32_t index);

// Supporting macro that makes it easier to pass non-pointer components
#ifndef ComponentAddType
#define ComponentAddType(component, typestr) ComponentAddTypePtr(&component, typestr)
#endif

// Supporting macro that makes it easier to pass non-pointer components
#ifndef ComponentRemoveType
#define ComponentRemoveType(component, index) ComponentRemoveTypePtr(&component, index)
#endif

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
// Serial Entities ////////////////////
///////////////////////////////////////

typedef struct Entity
{
    char name[MAX_STR_SIZE];
    Component components[MAX_COMPONENTS];
    uint32_t numComponents;
} Entity;

typedef struct DynamicEntityArray
{
    Arena *arena;
    Entity *array;
    uint32_t count;
    uint32_t capacity;
} DynamicEntityArray;

DynamicEntityArray *DynamicEntityAlloc();

// // Initializes entity and allocates memory for its components
// // returns false if allocation failed
// bool EntityInit(Arena *arena, Entity *entity, const char *name);

int32_t AddSerialEntity(DynamicEntityArray *entities, char *name);

void RemoveSerialEntity(DynamicEntityArray *entities, uint32_t index);

// // Add a component to an entity, takes a pointer
// void EntityAddComponentPtr(Entity *entity, Component component);

// Supporting macro that makes it easier to pass non-pointer components
// #ifndef EntityAddComponent
// #define EntityAddComponent(entity, component) EntityAddComponentPtr(&entity, component);
// #endif

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////

#endif
