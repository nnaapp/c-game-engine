#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>
#include "../include/arena.h"

#define EVENT_VALUE_SIZE 128

typedef struct Event
{
    uint32_t id;
    char *strValue;
    uint32_t intValue;
} Event;

typedef struct EventHashmap
{
    uint32_t **hashmap;
    uint32_t *sizes;
} EventHashmap;

typedef struct EventPool
{
    Event *events;
    uint32_t maxEvents;
    uint32_t numEvents;
    EventHashmap IDtoIndices; // Hashmap using buckets that associates event IDs with indices in the event array
} EventPool;

// Initialized event pool with default values and given max size,
// uses arena allocator passes as argument instead of malloc
bool EventPoolInit(EventPool *events, Arena *arena, uint32_t maxEvents);

// Publishes an event onto the event pool queue, uses arguments to format new event,
// strvalue size limit set at EVENT_VALUE_SIZE defined at the top of event.h
void EventPoolPublish(EventPool *events, uint32_t eventID, char *strValue, uint32_t intValue);

// Intended for use at end of frame, swaps current events with queued events,
// so the old events are overwritten and the queued events are processed next frame
void EventPoolIterate(EventPool *events);

// Subscribes to an event type, searching the current events array
// for the subscribed event ID
//
// Fow now, duplicate event types are ignored
// This can be changed if/when multiple events with the same ID need to be handled
// 
// Returns -1 when event could not be found
uint32_t *EventPoolSubscribe(EventPool *events, uint32_t eventID);

#endif
