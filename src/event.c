#include <stdlib.h>
#include <stdalign.h>
#include <string.h>
#include "../include/event.h"

// Initialized event pool with default values and given max size,
// uses arena allocator passes as argument instead of malloc
bool EventPoolInit(EventPool *events, Arena *arena, uint32_t maxEvents)
{
    events->events = PushArray(arena, Event, maxEvents);
    events->IDtoIndices.hashmap = PushArray(arena, EventHashmap *, maxEvents);
    events->IDtoIndices.sizes = PushArray(arena, uint32_t, maxEvents);
    for (int i = 0; i < maxEvents; i++)
    {
        events->events[i].strValue = PushArray(arena, char, EVENT_VALUE_SIZE);
        events->IDtoIndices.hashmap[i] = PushArray(arena, EventHashmap, maxEvents);
        events->IDtoIndices.sizes[i] = 0;
    }
    events->maxEvents = maxEvents;
    events->numEvents = 0;

    return true;
}

// Publishes an event onto the event pool queue, uses arguments to format new event,
// strvalue size limit set at EVENT_VALUE_SIZE defined at the top of event.h
void EventPoolPublish(EventPool *events, uint32_t eventID, char *strValue, uint32_t intValue)
{
    events->events[events->numEvents].id = eventID;
    events->events[events->numEvents].intValue = intValue;
    strcpy(events->events[events->numEvents].strValue, strValue);

    uint32_t size = events->IDtoIndices.sizes[eventID];
    events->IDtoIndices.hashmap[eventID][size] = events->numEvents;
    events->IDtoIndices.sizes[eventID]++;

    events->numEvents++;
}

// Intended for use at end of frame, swaps current events with queued events,
// so the old events are overwritten and the queued events are processed next frame
void EventPoolIterate(EventPool *events)
{
    events->numEvents = 0;
    for (int i = 0; i < events->maxEvents; i++)
    {
        events->IDtoIndices.sizes[i] = 0;
    }
}

// Subscribes to an event type, searching the current events array
// for the subscribed event ID

// Fow now, duplicate event types are ignored
// This can be changed if/when multiple events with the same ID need to be handled
// 
// Returns -1 when event could not be found
uint32_t *EventPoolSubscribe(EventPool *events, uint32_t eventID)
{
    if (events->IDtoIndices.sizes[eventID] == 0) return NULL;

    uint32_t *indices = malloc(sizeof(*indices) * events->IDtoIndices.sizes[eventID]);
    for (int i = 0; i < events->IDtoIndices.sizes[eventID]; i++)
    {
        indices[i] = events->IDtoIndices.hashmap[eventID][i];
    }

    return indices;
}
