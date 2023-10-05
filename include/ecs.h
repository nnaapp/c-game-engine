#ifndef ECS_H
#define ECS_H

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "../include/arena.h"
#include "event.h"

////
// TODO:
//      Documentation, comments/readme.md
//
//      Error handling
//
//      Int or bool return values indicating success or fail
//
//      Revert state on failure (dont change anything if something fails halfway through)
//
//      Possibly reduce overuse of uint32_t, maybe introduce some size_t
//
//      Maybe translate -1 underflow returns to true/false returns for ease of use on user end
//
//      Increase usability and readabiliy, functionize some things currently done maunally,
//          maybe turn the triple if(BITTEST...) sets into a function of some sort.
//          Maybe do it by making one Bitset and feeding it to a function that compares?
//          Also look into automatically handling component removal, instead of
//          having to call ECSClearComponent or ECSRemoveEntity and THEN using the macro a bunch of times.


// Tree of ECS members and what they do
// 
// ECS
// - EntityData
//     - maxEntities
//     - currentEntities
//     - IDQueue 
//         - ring buffer of available IDs
//     - Bitset *
//         - array of signatures, index in char array represents component ID,
//           set bit means entity has that type, un-set bit means entity does not
// - ComponentDict
//     - Remember, each ComponentDict is associated to a component type, these are registered at runtime,
//       component ID given by ECS is used as index in ComponentDict array
//     - indexToEntity
//         - Simple hashmap, key is index of component in component array, value is entity ID that owns it
//     - entityToIndex
//         - Simple hashmap, key is ID of entity, value is index of component it owns in component array 
//     - size (amount of entities that own that component type)
// - MaxComponents
// - CurrentComponents


///////////////////////////////////////
/// Array Queue (For entitiy IDs) /////
///////////////////////////////////////

// Ring buffer, used for entity IDs, ideal because
// the number of entity IDs is constant for any given ECS
typedef struct IDQueue
{
    uint32_t *arr;
    uint32_t capacity; // Max ids it can hold
    uint32_t size;   // Current number of ids it is holding
    uint32_t head;
    uint32_t tail;
} IDQueue;

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
/// Bitset (For entity signatures) ////
///////////////////////////////////////

#define BITMASK(bit) (1 << ((bit) & CHAR_BIT))
#define BITSLOT(bit) ((bit) / CHAR_BIT)
#define BITSET(arr, i) ((arr)[BITSLOT(i)] |= BITMASK(i))
#define BITCLEAR(arr, i) ((arr)[BITSLOT(i)] &= ~BITMASK(i))
#define BITTEST(arr, i) ((arr)[BITSLOT(i)] & BITMASK(i))
#define BITNSLOTS(nbits) ((nbits + CHAR_BIT - 1) / CHAR_BIT)

// Bitset ported from C++, used to store signatures for entities,
// which are used to track what components they do or don't have
typedef struct Bitset
{
    char *bits;
    uint32_t size;
} Bitset;

// Initialize BitSet struct, allocated onto given arena
// 
// Return - Boolean for success or failure
bool InitBitset(Bitset *b, Arena *mem, uint32_t size);

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
/// EntityData ////////////////////////
///////////////////////////////////////

// Stores current/max number of entities, a queue of available IDs,
// and an array of Bitset signatures to track what components an entity possesses
typedef struct EntityData
{
    uint32_t currentEntities;
    uint32_t maxEntities;
    IDQueue eIDs;
    Bitset *eSignatures;
} EntityData;

// Initialize EntityData struct, allocated onto given arena
//
// Return - Boolean for success or failure
bool InitEntityData(EntityData *m, Arena *mem, uint32_t maxEntities);

// Set signature for entity in EntityData struct, using given Bitset
//
// Return - Boolean for success or failure
bool SetSignature(EntityData *m, uint32_t entity, Bitset signature);

// Get signature for entity from EntityData struct
//
// Return - Bitset signature
Bitset *GetSignature(EntityData *m, uint32_t entity);

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
/// ComponentDict /////////////////////
///////////////////////////////////////

// Represents the associations between entities and component,
// for ONE component type, as well as the amount of entities that posess that component
typedef struct ComponentDict
{
    uint32_t *entityToIndex;
    uint32_t *indexToEntity;
    // Current number of entries
    uint32_t size;
} ComponentDict;

// Initialize ComponentDict struct, allocated onto given arena
//
// Return - Boolean for success or failure
bool InitComponentDict(ComponentDict *c, Arena *mem, uint32_t maxEntities);

// Add an entity to a ComponentDict
//
// Return - Boolean for success or failure
bool AddComponentDict(uint32_t entity, ComponentDict *container);

// Remove an entity from a ComponentDict
//
// Return - Boolean for success or failure
bool RemoveComponentDict(uint32_t entity, ComponentDict *container);
///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
/// Entity Component System ///////////
///////////////////////////////////////

// Contains all entity data, component-entity associations, and counts of components/entities,
// but does NOT contain any actual component data
typedef struct ECS
{
    EntityData entities;
    ComponentDict *components;
    uint32_t maxComponents;
    uint32_t currentComponents;
} ECS;

// Initializes ECS struct, allocates on the given arena
//
// Return - Boolean for success or failure
bool ECSInit(ECS *ecs, Arena *mem, uint32_t maxEntities, uint32_t maxComponents);

// Create an entity/ID
// 
// Return - uint32_t entity ID
uint32_t CreateEntity(ECS *ecs);

// Remove an entity/ID, set signature to all 0's, remove all associations in ComponentDicts
//
// Return - Boolean for success or failure
bool RemoveEntity(uint32_t entity, ECS *ecs);

// Associate an entity with a component ID, where the ID serves as an index in the ECS ComponentDict array
//
// Return - Boolean for success or failure
bool AssociateComponent(uint32_t entity, ECS *ecs, uint32_t componentID);

// Remove association between an entity and component ID, where the ID serves as an index in the ECS ComponentDict array
//
// Return - Boolean for success or failure
bool UnassociateComponent(uint32_t entity, ECS *ecs, uint32_t componentID);

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////


///////////////////////////////////////
/// ECS Usage Macros //////////////////
///////////////////////////////////////

#ifndef RegisterComponent
// Register component with the ECS, allocates the array AND sets the ID given by ECS
#define RegisterComponent(ecsptr, arena, componentSet, componentType){\
    componentSet.set = PushArray(arena, componentType, ecsptr->entities.maxEntities);\
    componentSet.id = ecsptr->currentComponents++;\
}
#endif

#ifndef AddComponent
// Associate component with entity in ECS, add data to appropriate index in array
#define AddComponent(entity, componentSet, ecsptr, data) {\
    AssociateComponent(entity, ecsptr, componentSet.id);\
    componentSet.set[GetEntityIndex(ecsptr, entity, componentSet.id)] = data;\
}
#endif

#ifndef RemoveComponent
// Unassociate component with entity in ECS, remove data from approprivate index in array
#define RemoveComponent(componentSet, ecsptr, entity) {\
    do {\
        uint32_t removed = ecs->components[componentSet.id].entityToIndex[entity];\
        uint32_t last = ecs->components[componentSet.id].size - 1;\
        componentSet.set[removed] = componentSet.set[last];\
    } while(0);\
}
#endif

#ifndef GetEntityID
// Gets ID of entity for given index in component array
#define GetEntityID(ecsptr, componentIndex, componentID) ecsptr->components[componentID].indexToEntity[componentIndex]
#endif

#ifndef GetEntityIndex
// Gets index in component array for given entity and component IDs
#define GetEntityIndex(ecsptr, entityID, componentID) ecsptr->components[componentID].entityToIndex[entityID]
#endif

#ifndef GetEntitySignature
// Gets Bitset signature for a given entity ID 
#define GetEntitySignature(ecsptr, entityID) ecsptr->entities.eSignatures[entityID]
#endif

///////////////////////////////////////
///////////////////////////////////////
///////////////////////////////////////

#endif
