#include "../include/serialeditor.h"
#include <string.h>

///////////////////////////////////////
// Serial Components //////////////////
///////////////////////////////////////

bool ComponentInit(Arena *arena, Component *component, const char *name, uint32_t maxMembers)
{
    component->maxMembers = maxMembers;
    component->numMembers = 0;

    component->name = PushArray(arena, char, MAX_STR_SIZE);
    strcpy(component->name, name);

    component->types = PushArray(arena, char *, maxMembers);
    if (component->types == NULL) return false;

    for (int i = 0; i < maxMembers; i++)
    {
        component->types[i] = PushArray(arena, char, MAX_STR_SIZE);
        if (component->types[i] == NULL) return false;
    }

    return true;
}

bool ComponentAddTypePtr(Component *component, const char *type)
{
    if (component->numMembers == component->maxMembers) return false;

    strcpy(component->types[component->numMembers], type);
    component->numMembers++;

    return true;
}

bool ComponentRemoveTypePtr(Component *component, uint32_t index)
{
    if (component->numMembers == 0 || index >= component->numMembers) return false;

    for (int i = index; i < component->numMembers - 1; i++)
    {
        component->types[i] = component->types[i + 1];
    }
    component->numMembers--;

    return true;
}

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////

///////////////////////////////////////
// Serial Entities ////////////////////
///////////////////////////////////////

DynamicEntityArray *DynamicEntityAlloc()
{
    DynamicEntityArray *arr = (DynamicEntityArray *)malloc(sizeof(DynamicEntityArray));
    arr->arena = ArenaAlloc();
    arr->array = (Entity *)arr->arena->arena;
    arr->count = 0;
    arr->capacity = 0;

    return arr;
}

// bool EntityInit(Arena *arena, Entity *entity, const char *name)
// {
//     entity->name = PushArray(arena, char, MAX_STR_SIZE);
//     if (entity->name == NULL) return false;
//     strcpy(entity->name, name);

//     entity->components = PushArray(arena, Component, MAX_COMPONENTS);
//     if (entity->components == NULL) return false;

//     entity->numComponents = 0;

//     return true;
// }

int32_t AddSerialEntity(DynamicEntityArray *entities, char *name)
{
    if (entities->count >= entities->capacity)
    {
        PushStruct(entities->arena, Entity);
        entities->capacity++;
    }
    strcpy(entities->array[entities->count].name, name);

    return entities->count++;
}

void RemoveSerialEntity(DynamicEntityArray *entities, uint32_t index)
{
    if (index >= entities->count) return;
    
    for (int i = index; i < entities->count - 1; i++)
    {
        entities->array[i] = entities->array[i + 1];
    }   

    entities->count--;
}

// void EntityAddComponentPtr(Entity *entity, Component component)
// {
//     entity->components[entity->numComponents] = component;
//     entity->numComponents++;
// }

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
