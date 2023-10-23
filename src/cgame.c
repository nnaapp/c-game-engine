#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "../include/raylib.h"
#include "../include/raymath.h"
#include "../include/ecs.h"
#include "../include/event.h"
#include "../include/components.h"
#include "../include/arena.h"
#include "../include/console.h"
#include "../include/util.h"

// TODO:
//  Restarting the game or quitting depending on player input, upon death
//  Ingame console log for game engine
//  Consider rectangular windows dynamically working, but thats secondary at best
//  Font size doesnt scale with window, consider fixing
//  Fix magic number constants
//  Make first snake food spawn randomly

#define SUCCESS_RETURN 0
#define FAIL_RETURN 1
#define RESTART_RETURN 2

#define TICKS_PER_SEC 8

#define DEBUG_FONT 20
#define GAME_FONT 50
#define DEBUG_TEXT_COLOR DARKGREEN
#define GAME_TEXT_COLOR BLACK
#define SNAKE_COLOR BLACK
#define BACKGROUND_COLOR (Color){171,217,154,255}
#define GRID_COLOR (Color){157,196,145,255}
#define FOOD_COLOR BLACK

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define BOARD_WIDTH 40
#define BOARD_HEIGHT 40
#define SEGMENT_SCALE 0.9

#define MAX_ENTITIES 65536
#define MAX_COMPONENTS 7
#define MAX_EVENTS 4

// Segments of the snakes body
typedef struct Segments
{
    uint32_t *entityIDs;
    uint32_t count;
} Segments;

// Event enum for event system
typedef enum EventTypes
{
    FoodEaten = 0,
    PlayerDied = 1
} EventTypes;

// Basic tilemap struct
typedef struct Tilemap
{
    uint32_t width;
    uint32_t height;
    uint32_t cellSize;
    bool *map;
} Tilemap;

int GameLoop(const int, const int);
void DrawSystem(ECS *, DrawRectSet, TextSet, PositionSet);
void CollectibleSystem(ECS *, EventPool *, Tilemap, uint32_t, CollectibleSet, PositionSet, ColliderSet, DrawRectSet);
void PlayerMovementSystem(ECS *, EventPool *, uint32_t, Tilemap, ControllerSet, PositionSet, ColliderSet);
void FollowSystem(ECS *, uint32_t, Tilemap, ControllerSet, PositionSet, FollowerSet);
void FoodEatenSystem(ECS *, EventPool *, Tilemap, uint32_t, Segments *, FollowerSet, CollectibleSet, PositionSet, DrawRectSet);
void SnakeCollideSystem(ECS *, EventPool *, uint32_t, Segments *, ColliderSet, PositionSet);
bool PlayerDeathSystem(ECS *, EventPool *, Console *, TextSet, PositionSet, const uint32_t, const uint32_t);

// Manages the state of the program and window
int main(int argc, char **argv)
{
    const int screenW = SCREEN_WIDTH;
    const int screenH = SCREEN_HEIGHT;

    srand(time(NULL));
    InitWindow(screenW, screenH, "Test Window");
    SetTargetFPS(60);

    int gameStatus;
    do
    {
        gameStatus = GameLoop(screenW, screenH);
    } while(gameStatus == RESTART_RETURN);

    return gameStatus;
}

// Manages the systems within the game window
int GameLoop(const int screenW, const int screenH)
{   
    // ECS Initialization
    uint32_t maxEntities = MAX_ENTITIES;
    uint32_t maxComponents = MAX_COMPONENTS;

    Arena *ecsArena = ArenaAlloc();

    // Initialize and allocate ECS all in the same arena,
    // tying the lifetimes of every piece of the ECS together
    ECS *ecs = PushStruct(ecsArena, ECS);
    ECSInit(ecs, ecsArena, maxEntities, maxComponents);

    Arena *componentArena = ArenaAlloc();

    // Component Initialization
    DrawRectSet drawRects;
    RegisterComponent(ecs, componentArena, drawRects, DrawRect);

    PositionSet positions;
    RegisterComponent(ecs, componentArena, positions, Position);

    ColliderSet colliders;
    RegisterComponent(ecs, componentArena, colliders, Collider);

    CollectibleSet collectibles;
    RegisterComponent(ecs, componentArena, collectibles, Collectible);

    ControllerSet controls;
    RegisterComponent(ecs, componentArena, controls, Controller);

    FollowerSet followers;
    RegisterComponent(ecs, componentArena, followers, Follower);

    TextSet texts;
    RegisterComponent(ecs, componentArena, texts, Text);

    // General use arena initialization
    Arena *generalArena = ArenaAlloc();

    // Console
    Console console;
    InitConsole(&console, generalArena, (Rectangle){0, 0, screenW, screenH}, 256, 256, 25, (Color){0,0,0,128}, GREEN);
    ConsoleSetKeys(&console, KEY_UP, KEY_DOWN, KEY_C);
    console.enabled = false;
    //ConsoleSetOutline(&console, -25, -25, screenW/2 + 25, screenH/2 + 25, 25, DARKGRAY);

    for (int i = 0; i < 30; i++)
    {
        char buff[8];
        sprintf(buff, "%d", i);
        WriteConsole(&console, buff);
    }

    // Board 
    Tilemap board;
    // Only works with square windows
    board.width = BOARD_WIDTH;
    board.height = BOARD_HEIGHT;
    board.cellSize = screenW / board.width;
    uint32_t boardArea = board.width * board.height;
    board.map = PushArray(generalArena, bool, boardArea);
    Line *backgroundGrid = PushArray(generalArena, Line, (board.width + board.height));
    int lineIndex = 0;
    for (int i = 0; i < board.width; i++)
    {
        Line *curLine = &backgroundGrid[lineIndex];
        curLine->thickness = 2;
        curLine->start.x = board.cellSize * i;
        curLine->start.y = 0;
        curLine->end.x = board.cellSize * i;
        curLine->end.y = board.height * board.cellSize;
        curLine->color = GRID_COLOR;
        lineIndex++;
    }
    for (int i = 0; i < board.height; i++)
    {
        Line *curLine = &backgroundGrid[lineIndex];
        curLine->thickness = 2;
        curLine->start.x = 0;
        curLine->start.y = board.cellSize * i;
        curLine->end.x = board.width * board.cellSize;
        curLine->end.y = board.cellSize * i;
        curLine->color = GRID_COLOR;
        lineIndex++;
    }
    
    // Events 
    EventPool eventPool;
    uint32_t eventTypes = MAX_EVENTS;
    EventPoolInit(&eventPool, generalArena, eventTypes);

    // Snake player initialization
    uint32_t snakeID = CreateEntity(ecs);

    float dimension = board.cellSize * SEGMENT_SCALE;
    float tileOffset = (board.cellSize - dimension) / 2;
    Vector2 startWorld = { board.cellSize * ((float)board.width / 2.0f) + tileOffset, board.cellSize * ((float)board.height / 2.0f) + tileOffset };
    Vector2Int startBoard = { board.width / 2, board.height / 2 };
    
    AddComponent(snakeID, positions, ecs, ((Position){ startWorld, startBoard, startWorld, startBoard }));
    board.map[startBoard.x + (startBoard.y * board.width)] = true;

    DrawRect snakeRect;
    snakeRect.rect = (Rectangle){ startWorld.x, startWorld.y, dimension, dimension };
    snakeRect.color = SNAKE_COLOR;
    AddComponent(snakeID, drawRects, ecs, snakeRect);

    AddComponent(snakeID, colliders, ecs, ((Collider){ true, snakeRect.rect }));

    AddComponent(snakeID, controls, ecs, ((Controller){ KEY_A, KEY_D, KEY_W, KEY_S, -1 }));

    // Snake follower array initialization
    Segments segments;
    segments.entityIDs = PushArray(generalArena, uint32_t, boardArea - 1);
    segments.count = 0;

    // Initial food collectible
    uint32_t initialFood = CreateEntity(ecs);

    Vector2Int foodBoardPos;
    do
    {
        foodBoardPos = (Vector2Int){ rand() % board.width, rand() % board.width };
    } while(foodBoardPos.x == startBoard.x && foodBoardPos.y == startBoard.y);
    Vector2 foodWorldPos = 
            { foodBoardPos.x * board.cellSize + (float)board.cellSize / 4.0f, foodBoardPos.y * board.cellSize + (float)board.cellSize / 4.0f };
    AddComponent(initialFood, positions, ecs, ((Position){ foodWorldPos, foodBoardPos }));

    Rectangle foodRect = { foodWorldPos.x, foodWorldPos.y, (float)board.cellSize / 2.0f, (float)board.cellSize / 2.0f };
    DrawRect foodDrawRect = { foodRect, FOOD_COLOR };
    AddComponent(initialFood, drawRects, ecs, foodDrawRect);

    AddComponent(initialFood, collectibles, ecs, ((Collectible){ FoodEaten }));

    // Gameplay loop
    bool runSystems = true;
    float tickTimer = 0.0f;
    float tickMaxTime = 1.0f / (float)TICKS_PER_SEC;
    while(!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        tickTimer += deltaTime;

        if (runSystems && tickTimer >= tickMaxTime)
        {
            tickTimer -= tickMaxTime;

            // Systems
            PlayerMovementSystem(ecs, &eventPool, snakeID, board, controls, positions, colliders);
            CollectibleSystem(ecs, &eventPool, board, snakeID, collectibles, positions, colliders, drawRects);
            FollowSystem(ecs, snakeID, board, controls, positions, followers);
            SnakeCollideSystem(ecs, &eventPool, snakeID, &segments, colliders, positions);

            // Event Handlers
            FoodEatenSystem(ecs, &eventPool, board, snakeID, &segments, followers, collectibles, positions, drawRects);
            runSystems = PlayerDeathSystem(ecs, &eventPool, &console, texts, positions, screenW, screenH);

            // Clear event pool at end of tick
            EventPoolIterate(&eventPool);
        }

        if (!runSystems)
        {
            if (IsKeyPressed(KEY_R))
            {
                fprintf(stderr, "Freeing memory.\n");
                ArenaDealloc(generalArena);
                ArenaDealloc(ecsArena);
                ArenaDealloc(componentArena);
                return RESTART_RETURN;
            }
        }

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        for (int i = 0; i < board.width + board.height; i++)
        {
            DrawLineEx(backgroundGrid[i].start, backgroundGrid[i].end, backgroundGrid[i].thickness, backgroundGrid[i].color);
        }

        // Debug info
        char fpsbuf[16];
        sprintf(fpsbuf, "FPS : %d", GetFPS());
        DrawText(fpsbuf, screenW * 0.05f, screenH * 0.05f, DEBUG_FONT, DEBUG_TEXT_COLOR);

        char dtbuf[16];
        sprintf(dtbuf, "dT : %.5f", deltaTime);
        DrawText(dtbuf, screenW * 0.05f, screenH * 0.05f + 20, DEBUG_FONT, DEBUG_TEXT_COLOR);

        char entitybuf[32];
        sprintf(entitybuf, "Entities : %d", ecs->entities.currentEntities);
        DrawText(entitybuf, screenW * 0.05f, screenH * 0.05f + 40, DEBUG_FONT, DEBUG_TEXT_COLOR);

        char ecsPageBuf[32];
        sprintf(ecsPageBuf, "ECS Pages : %d", ecsArena->pages);
        DrawText(ecsPageBuf, screenW * 0.05f, screenH * 0.05f + 60, DEBUG_FONT, DEBUG_TEXT_COLOR);

        char componentPageBuf[32];
        sprintf(componentPageBuf, "Comp. Pages : %d", componentArena->pages);
        DrawText(componentPageBuf, screenW * 0.05f, screenH * 0.05f + 80, DEBUG_FONT, DEBUG_TEXT_COLOR);

        char generalPageBuf[32];
        sprintf(generalPageBuf, "Gen. Pages : %d", generalArena->pages);
        DrawText(generalPageBuf, screenW * 0.05f, screenH * 0.05f + 100, DEBUG_FONT, DEBUG_TEXT_COLOR);
        //
        
        DrawSystem(ecs, drawRects, texts, positions);

        ConsoleUpdate(&console);

        EndDrawing();
    }

    CloseWindow();

    fprintf(stderr, "Freeing memory.\n");
    ArenaDealloc(generalArena);
    ArenaDealloc(ecsArena);
    ArenaDealloc(componentArena);

    return SUCCESS_RETURN;
}

// Only call after BeginDrawing() has been called, and before drawing is done
void DrawSystem(ECS *ecs, DrawRectSet drawRects, TextSet texts, PositionSet pos)
{
    for (int i = 0; i < ecs->components[drawRects.id].size; i++)
    {
        uint32_t entityID = GetEntityID(ecs, i, drawRects.id);
        if (!BITTEST(GetEntitySignature(ecs, entityID).bits, drawRects.id)) continue;
        if (!BITTEST(GetEntitySignature(ecs, entityID).bits, pos.id)) continue;
        

        uint32_t positionIndex = GetEntityIndex(ecs, entityID, pos.id);

        drawRects.set[i].rect.x = pos.set[positionIndex].world.x;
        drawRects.set[i].rect.y = pos.set[positionIndex].world.y;

        DrawRect cur = drawRects.set[i];
        DrawRectangleRec(cur.rect, cur.color);
    }

    for (int i = 0; i < ecs->components[texts.id].size; i++)
    {
        uint32_t entityID = GetEntityID(ecs, i, texts.id);
        if (!BITTEST(GetEntitySignature(ecs, entityID).bits, texts.id)) continue;
        if (!BITTEST(GetEntitySignature(ecs, entityID).bits, pos.id)) continue;

        uint32_t positionIndex = GetEntityIndex(ecs, entityID, pos.id);

        Text curText = texts.set[i];
        Position curPos = pos.set[positionIndex];
        DrawText(curText.text, curPos.world.x, curPos.world.y, curText.fontSize, curText.color);
    }

    return;
}

void CollectibleSystem(ECS *ecs, EventPool *events, Tilemap tilemap, uint32_t playerID, CollectibleSet collect, PositionSet pos, ColliderSet collide, DrawRectSet draw)
{
    char *playerSignature = GetEntitySignature(ecs, playerID).bits;
    if (!BITTEST(playerSignature, collide.id)) return;

    uint32_t playerColliderIndex = GetEntityIndex(ecs, playerID, collide.id);

    Rectangle playerRect = collide.set[playerColliderIndex].rect;

    uint32_t *toRemove = malloc(sizeof(*toRemove) * ecs->components[collect.id].size);
    uint32_t removeCount = 0;
    for (int i = 0; i < ecs->components[collect.id].size; i++)
    {
        uint32_t entityID = GetEntityID(ecs, i, collect.id);
        char *entitySignature = GetEntitySignature(ecs, entityID).bits;
        if (!BITTEST(entitySignature, collect.id)) continue;
        if (!BITTEST(entitySignature, pos.id)) continue;

        uint32_t positionIndex = GetEntityIndex(ecs, entityID, pos.id);

        // Find if player is contacting a collectible
        Vector2 collectPos = pos.set[positionIndex].world;

        bool xAlign = (playerRect.x <= collectPos.x && playerRect.x + playerRect.width > collectPos.x);
        bool yAlign = (playerRect.y <= collectPos.y && playerRect.y + playerRect.height >= collectPos.y);

        // Run function ptr inside of touched collectible
        if (xAlign && yAlign)
        {
            EventPoolPublish(events, collect.set[entityID].event, "", 0);
            tilemap.map[pos.set[positionIndex].tile.x + (pos.set[positionIndex].tile.y * tilemap.width)] = false;
            toRemove[removeCount] = entityID;
            removeCount++;
        }
    }

    for (int i = 0; i < removeCount; i++)
    {
        RemoveComponent(collect, ecs, toRemove[i]);
        RemoveComponent(pos, ecs, toRemove[i]);
        RemoveComponent(draw, ecs, toRemove[i]);
        RemoveEntity(toRemove[i], ecs);
    }

    free(toRemove);
}

void PlayerMovementSystem(ECS *ecs, EventPool *events, uint32_t playerID, Tilemap tilemap, ControllerSet control, PositionSet pos, ColliderSet collide)
{
    // Signature validation
    char *playerSignature = GetEntitySignature(ecs, playerID).bits;
    if (!BITTEST(playerSignature, control.id)) return;
    if (!BITTEST(playerSignature, pos.id)) return;
    if (!BITTEST(playerSignature, collide.id)) return;

    uint32_t controlsIndex = GetEntityIndex(ecs, playerID, control.id);
    uint32_t positionsIndex = GetEntityIndex(ecs, playerID, pos.id);
    uint32_t collideIndex = GetEntityIndex(ecs, playerID, collide.id);

    Controller *playerControl = &control.set[controlsIndex];
    Position *playerPos = &pos.set[positionsIndex];

    uint16_t left, right, up, down;
    left = playerControl->left;
    right = playerControl->right;
    up = playerControl->up;
    down = playerControl->down;

    uint32_t curDirection = playerControl->direction;
    int32_t polarity;
    bool movingX = false;
    bool movingY = false;

    // Priority for new inputs, with constraint to prevent doubling back on yourself
    if (IsKeyDown(left) && left != curDirection && curDirection != right)
    {
        curDirection = playerControl->left;
    }
    else if (IsKeyDown(right) && right != curDirection && curDirection != left)
    {
        curDirection = playerControl->right;
    }
    else if (IsKeyDown(up) && up != curDirection && curDirection != down)
    {
        curDirection = playerControl->up;
    }
    else if (IsKeyDown(down) && down != curDirection && curDirection != up)
    {
        curDirection = playerControl->down;
    }

    // If no input, check for existing direction
    if (curDirection == left)
    {
        movingX = true;
        polarity = -1;
    }
    else if (curDirection == right)
    {
        movingX = true;
        polarity = 1;
    }
    else if (curDirection == up)
    {
        movingY = true;
        polarity = -1;
    }
    else if (curDirection == down)
    {
        movingY = true;
        polarity = 1;
    }

    if (curDirection == -1) return;

    bool wallCollision = false;
    // If wall would be collided with in X direction, set flag
    if (movingX && ((playerPos->tile.x + polarity < 0) || (playerPos->tile.x + polarity >= tilemap.width)))
        wallCollision = true;

    // If wall would be collided with in Y direction, set flag
    if (movingY && ((playerPos->tile.y + polarity < 0) || (playerPos->tile.y + polarity >= tilemap.height)))
        wallCollision = true;

    // If wall would be collided with, kill the player
    if (wallCollision)
    {
        EventPoolPublish(events, PlayerDied, "Player died via wall collision.", 0);
        playerControl->direction = -1;
        return;
    }

    // If no collision, move the player
    playerPos->prevWorld = playerPos->world;
    playerPos->prevTile = playerPos->tile;

    tilemap.map[playerPos->tile.x + (playerPos->tile.y * tilemap.width)] = false;

    if (movingX)
    {
        playerPos->tile.x += polarity;
        playerPos->world.x += (int32_t)tilemap.cellSize * polarity;
    }

    if (movingY)
    {
        playerPos->tile.y += polarity;
        playerPos->world.y += (int32_t)tilemap.cellSize * polarity;
    }

    playerControl->direction = curDirection;
    tilemap.map[playerPos->tile.x + (playerPos->tile.y * tilemap.width)] = true;

    collide.set[collideIndex].rect.x = playerPos->world.x;
    collide.set[collideIndex].rect.y = playerPos->world.y;
}

void FollowSystem(ECS *ecs, uint32_t playerID, Tilemap tilemap, ControllerSet control, PositionSet pos, FollowerSet follow)
{
    char *playerSignature = GetEntitySignature(ecs, playerID).bits;
    if (!BITTEST(playerSignature, pos.id)) return;
    if (!BITTEST(playerSignature, control.id)) return;

    uint32_t playerControlIndex = GetEntityIndex(ecs, playerID, control.id);
    if (control.set[playerControlIndex].direction == -1) return;

    for (int i = 0; i < ecs->components[follow.id].size; i++)
    {
        uint32_t entityID = GetEntityID(ecs, i, follow.id);

        char *entitySignature = GetEntitySignature(ecs, entityID).bits;
        if (!BITTEST(entitySignature, follow.id)) continue;
        if (!BITTEST(entitySignature, pos.id)) continue;

        uint32_t positionIndex = GetEntityIndex(ecs, entityID, pos.id);

        uint32_t followedID = follow.set[i].followID;
        uint32_t followedPositionIndex = GetEntityIndex(ecs, followedID, pos.id);

        if (Vector2Compare(pos.set[positionIndex].world, pos.set[followedPositionIndex].prevWorld)) continue;

        Vector2Int oldTile = pos.set[positionIndex].tile;
        tilemap.map[oldTile.x + (oldTile.y * tilemap.width)] = false;

        pos.set[positionIndex].prevWorld = pos.set[positionIndex].world;
        pos.set[positionIndex].prevTile = pos.set[positionIndex].tile;

        pos.set[positionIndex].world = pos.set[followedPositionIndex].prevWorld;
        pos.set[positionIndex].tile = pos.set[followedPositionIndex].prevTile;

        Vector2Int newTile = pos.set[positionIndex].tile;
        tilemap.map[newTile.x + (newTile.y * tilemap.width)] = true;
    }
}

void SnakeCollideSystem(ECS *ecs, EventPool *events, uint32_t playerID, Segments *segments, ColliderSet collide, PositionSet pos)
{
    char *playerSignature = GetEntitySignature(ecs, playerID).bits;
    if (!BITTEST(playerSignature, collide.id)) return;

    uint32_t playerColliderIndex = GetEntityIndex(ecs, playerID, collide.id);

    Rectangle playerRect = collide.set[playerColliderIndex].rect;

    for (int i = 0; i < segments->count; i++)
    {
        uint32_t segmentID = segments->entityIDs[i];

        char *segmentSignature = GetEntitySignature(ecs, segmentID).bits;
        if (!BITTEST(segmentSignature, pos.id)) continue;

        uint32_t positionIndex = GetEntityIndex(ecs, segmentID, pos.id);

        Vector2 segmentPos = pos.set[positionIndex].world;

        bool xAlign = (playerRect.x <= segmentPos.x && playerRect.x + playerRect.width > segmentPos.x);
        bool yAlign = (playerRect.y <= segmentPos.y && playerRect.y + playerRect.height >= segmentPos.y);

        if (xAlign && yAlign)
        {
            EventPoolPublish(events, PlayerDied, "Player died via self collision.", 0);
            return;
        }
    }
}

void FoodEatenSystem(ECS *ecs, EventPool *events, Tilemap tilemap, uint32_t playerID, Segments *segments, FollowerSet follow, CollectibleSet collect, PositionSet pos, DrawRectSet draw)
{
    uint32_t *foodEatenIndex = EventPoolSubscribe(events, FoodEaten);
    if (foodEatenIndex == NULL) return;

    // Spawn new snake segment
    uint32_t newSegmentID = CreateEntity(ecs);

    Vector2 segmentWorld = { (float)-INT32_MAX, (float)-INT32_MAX };
    Vector2Int segmentTile = { 0, 0 };
    AddComponent(newSegmentID, pos, ecs, ((Position){ segmentWorld, segmentTile, segmentWorld, segmentTile }));

    DrawRect segmentRect;
    float segmentDimension = tilemap.cellSize * SEGMENT_SCALE;
    segmentRect.rect = (Rectangle){ segmentWorld.x, segmentWorld.y, segmentDimension, segmentDimension };
    segmentRect.color = SNAKE_COLOR;
    AddComponent(newSegmentID, draw, ecs, segmentRect);

    uint32_t idToFollow;
    if (segments->count == 0)
        idToFollow = playerID;
    else
        idToFollow = segments->entityIDs[segments->count - 1];

    AddComponent(newSegmentID, follow, ecs, ((Follower){ idToFollow }));

    segments->entityIDs[segments->count] = newSegmentID;
    segments->count++;

    // Spawn new food collectible
    uint32_t newFoodID = CreateEntity(ecs);

    uint32_t mapArea = tilemap.width * tilemap.height;
    uint32_t *validTiles = malloc(sizeof(*validTiles) * mapArea);
    uint32_t validTileCount = 0;
    for (int i = 0; i < mapArea; i++)
    {
        if (tilemap.map[i] == true) continue;

        validTiles[validTileCount] = i;
        validTileCount++;
    }

    uint32_t foodTile = validTiles[rand() % validTileCount] ;
    Vector2Int tileCoords = { foodTile % tilemap.width, foodTile / tilemap.width };
    uint32_t foodDimension = tilemap.cellSize / 2;
    Vector2 worldCoords = 
        { tileCoords.x * tilemap.cellSize + ((float)foodDimension / 2.0f), tileCoords.y * tilemap.cellSize + ((float)foodDimension / 2.0f) };
    AddComponent(newFoodID, pos, ecs, ((Position){ worldCoords, tileCoords }));

    DrawRect foodRect;
    foodRect.rect = (Rectangle){ worldCoords.x, worldCoords.y, foodDimension, foodDimension };
    foodRect.color = FOOD_COLOR;
    AddComponent(newFoodID, draw, ecs, foodRect);

    AddComponent(newFoodID, collect, ecs, (Collectible){ FoodEaten });

    free(validTiles);
    //
}

bool PlayerDeathSystem(ECS *ecs, EventPool *events, Console *log, TextSet text, PositionSet pos, const uint32_t screenW, const uint32_t screenH)
{
    uint32_t *playerDeathIndex = EventPoolSubscribe(events, PlayerDied);
    if (playerDeathIndex == NULL) return true;

    Event playerDeathEvent = events->events[*playerDeathIndex];
    if (strlen(playerDeathEvent.strValue) > 0)
        WriteConsole(log, playerDeathEvent.strValue);

    uint32_t gameOverText = CreateEntity(ecs);
    AddComponent(gameOverText, text, ecs, ((Text){ "Game Over", GAME_TEXT_COLOR, GAME_FONT }));
    Vector2 gameOverTextPos = { (float)screenW / 2.0f - ((float)MeasureText("Game Over", GAME_FONT) / 2.0f), (float)screenH / 3.0f };
    AddComponent(gameOverText, pos, ecs, ((Position){ gameOverTextPos }));

    uint32_t restartPrompt = CreateEntity(ecs);
    AddComponent(restartPrompt, text, ecs, ((Text){ "Press R to Restart", GAME_TEXT_COLOR, GAME_FONT / 2.0f }));
    Vector2 restartPos = { (float)screenW / 2.0f - ((float)MeasureText("Press R to Restart", GAME_FONT / 2.0f) / 2.0f), (float)screenH / 2.0f };
    AddComponent(restartPrompt, pos, ecs, ((Position){ restartPos }));

    return false;
}
