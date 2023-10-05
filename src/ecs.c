#include <string.h>
#include <limits.h>
#include <stdalign.h>
#include <stdlib.h>
#include "../include/ecs.h"

///////////////////////////////////////
/// Array Queue (For entitiy IDs) /////
///////////////////////////////////////

bool InitIDQueue(IDQueue *q, Arena *mem, uint32_t capacity)
{
    // TODO: error handling

    q->arr = PushArray(mem, uint32_t, capacity);
    if (q->arr == NULL) return false;

    q->capacity = capacity;
    q->size = 0;

    q->head = 0;
    q->tail = 0;

    return true;
}

bool IDEnqueue(IDQueue *q, uint32_t value)
{
    if (q->size >= q->capacity) return false;

    q->arr[q->tail] = value;
    q->size++;
    q->tail = (q->tail + 1) % q->capacity;

    return true;
}

bool IDDequeue(IDQueue *q)
{
    if (q->size == 0) return false;

    q->head = (q->head + 1) % q->capacity;
    q->size--;

    return true;
}

uint32_t IDQueuePeek(IDQueue *q)
{
    return q->size != 0 ? q->arr[q->head] : -1;
}

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////

///////////////////////////////////////
/// Bitset (For entity signatures) ////
///////////////////////////////////////

bool InitBitset(Bitset *b, Arena *mem, uint32_t size)
{
    b->size = BITNSLOTS(size);

    b->bits = PushArrayZero(mem, char, size);
    return (b->bits != NULL);
}

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
/// Entity Set ////////////////////////
///////////////////////////////////////

bool InitEntityData(EntityData *m, Arena *mem, uint32_t maxEntities)
{
    m->currentEntities = 0;
    m->maxEntities = maxEntities;
    
    if (!InitIDQueue(&m->eIDs, mem, maxEntities)) return false;
    for (int i = 0; i < maxEntities; i++)
    {
        // If queue was full and value not enqueued, return false
        if (!IDEnqueue(&m->eIDs, i)) return false;
    }
    
    m->eSignatures = PushArray(mem, Bitset, maxEntities);
    if (m->eSignatures == NULL) return false;

    return true;
}

bool SetSignature(EntityData *m, uint32_t entity, Bitset signature)
{
    if (entity >= m->maxEntities) return false;

    m->eSignatures[entity] = signature;

    return true;
}

Bitset *GetSignature(EntityData *m, uint32_t entity)
{
    if (entity >= m->maxEntities) return NULL;

    return &m->eSignatures[entity];
}

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
/// Entity Component Association //////
///////////////////////////////////////

// -1 used to denote non-associated indices
bool InitComponentDict(ComponentDict *c, Arena *mem, uint32_t maxEntities)
{
    // TODO: error handling
    c->size = 0;

    c->entityToIndex = PushArray(mem, uint32_t, maxEntities);
    memset(c->entityToIndex, -1, sizeof(uint32_t) * maxEntities);
    c->indexToEntity = PushArray(mem, uint32_t, maxEntities);
    memset(c->indexToEntity, -1, sizeof(uint32_t) * maxEntities);

    return true;
}

bool AddComponentDict(uint32_t entity, ComponentDict *container)
{
    // TODO: prevent double-adding, which is why this returns a bool.
    if (container->entityToIndex[entity] != -1) return false;
    uint32_t index = container->size;

    container->entityToIndex[entity] = index;
    container->indexToEntity[index] = entity;

    container->size++;

    return true;
}

bool RemoveComponentDict(uint32_t entity, ComponentDict *container)
{
    if (container->entityToIndex[entity] == -1) return false;

    uint32_t removedIndex = container->entityToIndex[entity];
    container->entityToIndex[entity] = -1;
    uint32_t lastIndex = container->size - 1;

    uint32_t lastEntity = container->indexToEntity[lastIndex];
    container->entityToIndex[lastEntity] = removedIndex;
    container->indexToEntity[removedIndex] = lastEntity;

    container->size--;

    return true;
}

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
/// Entity Component System ///////////
///////////////////////////////////////

bool ECSInit(ECS *ecs, Arena *mem, uint32_t maxEntities, uint32_t maxComponents)
{
    // TODO: error handling

    ecs->maxComponents = maxComponents;
    ecs->currentComponents = 0;

    // Arena allocation for component types
    ecs->components = PushArray(mem, ComponentDict, maxComponents);

    // Arena allocation for arrays within each component
    for (int i = 0; i < maxComponents; i++)
    {
        InitComponentDict(&ecs->components[i], mem, maxEntities);
    }

    // Arena allocation for entitiy set arrays
    InitEntityData(&ecs->entities, mem, maxEntities); // ARENA-FY THIS, currently memory leaks
    for (int i = 0; i < maxEntities; i++)
    {
        // Arena allocation for bitset arrays within entities
        InitBitset(&ecs->entities.eSignatures[i], mem, maxComponents);
    }

    return true;
}

uint32_t CreateEntity(ECS *ecs)
{
    EntityData *data = &ecs->entities;
    if (data->currentEntities >= data->maxEntities) return -1;

    uint32_t id = IDQueuePeek(&data->eIDs);
    if (id == -1) return -1;
    IDDequeue(&data->eIDs);
    data->currentEntities++;

    return id;
}

bool RemoveEntity(uint32_t entity, ECS *ecs)
{
    EntityData *data = &ecs->entities;
    if (entity >= data->maxEntities) return false;

    memset(data->eSignatures[entity].bits, 0, data->eSignatures[entity].size);

    if (!IDEnqueue(&data->eIDs, entity)) return false;
    data->currentEntities--;
   
    for (int i = 0; i < ecs->maxComponents; i++)
    {
        RemoveComponentDict(entity, &ecs->components[i]);
    }

    return true;
}

bool AssociateComponent(uint32_t entity, ECS *ecs, uint32_t componentID)
{
    // Update component set to reflect new component
    if (!AddComponentDict(entity, &ecs->components[componentID])) return false;
    // Update entity signature to reflect new component
    BITSET(ecs->entities.eSignatures[entity].bits, componentID);

    return true;
}

bool UnassociateComponent(uint32_t entity, ECS *ecs, uint32_t componentID)
{
    // Update component set to reflect removed component
    if (!RemoveComponentDict(entity, &ecs->components[componentID])) return false;
    // Update entity signature to reflect removed component
    BITCLEAR(ecs->entities.eSignatures[entity].bits, componentID);

    return true;
}

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////
