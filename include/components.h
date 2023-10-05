#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <stdint.h>
#include "../include/raylib.h"
#include "../include/util.h"

// Position
typedef struct Position
{
    Vector2 world;
    Vector2Int tile;
    Vector2 prevWorld;
    Vector2Int prevTile;
} Position;

typedef struct PositionSet
{
    Position *set;
    uint32_t id;
} PositionSet;
//

// PhysicsBody
typedef struct PhysicsBody
{
    Vector2 velocity;
    float xMax;
    float yMax;
    float gravity;
    float friction;
} PhysicsBody;

typedef struct PhysicsBodySet
{
    PhysicsBody *set;
    uint32_t id;
} PhysicsBodySet;
//

// DrawRect
typedef struct DrawRect
{
    Rectangle rect;
    Color color;
} DrawRect;

typedef struct DrawRectSet
{
    DrawRect *set;
    uint32_t id;
} DrawRectSet;
//

// Collider
typedef struct Collider
{
    bool dynamic;
    Rectangle rect;
} Collider;

typedef struct ColliderSet
{
    Collider *set;
    uint32_t id;
} ColliderSet;
//

// KineticControl
typedef struct KineticControl
{
    float speedV;
    float jumpV;
    uint16_t left;
    uint16_t right;
    uint16_t up;
    uint16_t down;
    bool grounded;
    bool horizontalInput;
} KineticControl;

typedef struct KineticControlSet
{
    KineticControl *set;
    uint32_t id;
} KineticControlSet;

// Collectible
typedef struct Collectible
{
    uint32_t event;
} Collectible;

typedef struct CollectibleSet
{
    Collectible *set;
    uint32_t id;
} CollectibleSet;

// Controller
typedef struct Controller
{
    uint16_t left;
    uint16_t right;
    uint16_t up;
    uint16_t down;
    // Direction will be -1 if stationary, 32 bit because clangd was complaining when it was 16?
    uint32_t direction;
} Controller;

typedef struct ControllerSet
{
    Controller *set;
    uint32_t id;
} ControllerSet;

// Follower
typedef struct Follower
{
    uint32_t followID;
} Follower;

typedef struct FollowerSet
{
    Follower *set;
    uint32_t id;
} FollowerSet;
//

// Text
typedef struct Text
{
    char *text;
    Color color;
    uint32_t fontSize;
} Text;

typedef struct TextSet
{
    Text *set;
    uint32_t id;
} TextSet;
//

#endif
