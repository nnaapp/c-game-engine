#include "../include/raylib.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../include/cimgui.h"
#define NO_FONT_AWESOME
#include "../include/rlImGui.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../include/util.h"
#include "../include/arena.h"
#include "../include/dynamicarray.h"

bool TileSelector();
void RenderGrid(Vector2Int dimensions, int cellSize, Color color);
void RenderTiles(Vector2Int dimensions, int32_t cellSize, DynamicTiles tilemap);
void CameraControls(Camera2D *camera);

int main(void)
{
    Vector2 displaySize = { 1024, 1024 };
    
    // Raylib window initialization
    InitWindow(displaySize.x, displaySize.y, "Tilemap Editor");
    SetTargetFPS(144);

    // cImGui and rlImGui setup
    rlImGuiSetup(true);

    Camera2D camera;
    camera.offset = (Vector2){ displaySize.x / 2, displaySize.y / 2 };
    camera.rotation = 0.0f;
    camera.target = (Vector2){ displaySize.x / 2, displaySize.y / 2 };
    camera.zoom = 1.0f;

    Vector2Int mapDimensions = { 24, 24 };
    DynamicTiles tilemap;
    InitDynamicArray(tilemap, Color);
    DynamicResize(tilemap, Color, mapDimensions.x * mapDimensions.y);
    for (int i = 0; i < mapDimensions.x * mapDimensions.y; i++)
    {
        AppendArrayDynamic(tilemap, BLACK);
    }
   
    while (!WindowShouldClose())
    {        
        // Begin raylib frame
        BeginDrawing();

        ClearBackground(DARKGRAY);

        // cImGui and rlImGui start frame
        rlImGuiBegin();

        // D for demo window (shows all the things ImGui can do)
        static bool showDemo = false;
        if (IsKeyPressed(KEY_GRAVE)) showDemo = !showDemo;
        if (showDemo) igShowDemoWindow(NULL);

        // Function for prototype tile selector
        TileSelector();

        // Start camera rendering
        CameraControls(&camera);
        BeginMode2D(camera);

        static const Color gridColor = { 219, 219, 219, 90 };
        static const int32_t cellSize = 32;
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        for (int i = 0; i < mapDimensions.y; i++)
        {
            for (int j = 0; j < mapDimensions.x; j++)
            {
                if (CheckCollisionPointRec(mouseWorldPos, (Rectangle){ j * cellSize, i * cellSize, cellSize, cellSize }))
                    tilemap.values[mapDimensions.x * i + j] = GREEN;
            }
        }
        RenderTiles(mapDimensions, cellSize, tilemap);
        RenderGrid(mapDimensions, cellSize, gridColor);

        EndMode2D();
        // End camera rendering

        // cImGui and rlImGui end frame
        rlImGuiEnd();
        // End raylib frame
        EndDrawing();
    }

    // cImGui and rlImGui shutdown
    rlImGuiShutdown();

    // Raylib window shutdown
    CloseWindow();
}

bool TileSelector()
{
    igBegin("Tile Selector Window", NULL, 0);

    igText("Tile Set Selector");

    static bool selected[9] = {true, false, false, false, false, false, false, false, false};
    static int currentSelect = 0;

    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            if (x > 0) igSameLine(0, -10.0f);
            
            igPushID_Int(3 * y + x);
            if (igSelectable_Bool("Tile", selected[3 * y + x], 0, (ImVec2){80, 80}))
            {
                
                selected[3 * y + x] ^= 1;
                if (selected[3 * y + x]) 
                { 
                    selected[currentSelect] = false;
                    currentSelect = 3 * y + x; 
                }
            }
            igPopID();
        }
    }

    igEnd();

    return currentSelect;
}

void RenderGrid(Vector2Int dimensions, int32_t cellSize, Color color)
{
    Vector2 start, end;   

    for (int i = 0; i <= dimensions.x; i++)
    {
        start.x = i * cellSize;
        start.y = 0;
        end.x = i * cellSize;
        end.y = dimensions.y * cellSize;
        DrawLineEx(start, end, 2, color);
    }

    for (int i = 0; i <= dimensions.y; i++)
    {
        start.x = 0;
        start.y = i * cellSize;
        end.x = dimensions.x * cellSize;
        end.y = i * cellSize;
        DrawLineEx(start, end, 2, color);
    }
}

void RenderTiles(Vector2Int dimensions, int32_t cellSize, DynamicTiles tilemap)
{
    for (int i = 0; i < dimensions.y; i++)
    {
        for (int j = 0; j < dimensions.x; j++)
        {
            DrawRectangle(j * cellSize, i * cellSize, cellSize, cellSize, tilemap.values[dimensions.x * i + j]);
        }
    }
}

void CameraControls(Camera2D *camera)
{
    const float moveSpeed = 5.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) camera->target.y -= moveSpeed;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) camera->target.y += moveSpeed;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) camera->target.x -= moveSpeed;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) camera->target.x += moveSpeed;

    const float zoomSpeed = 0.1f;
    if ((IsKeyPressed(KEY_SEMICOLON) || IsKeyDown(KEY_LEFT_BRACKET)) && camera->zoom - zoomSpeed > 0.1f) camera->zoom -= zoomSpeed;
    if (IsKeyPressed(KEY_APOSTROPHE) || IsKeyDown(KEY_RIGHT_BRACKET)) camera->zoom += zoomSpeed; 

    if (IsKeyPressed(KEY_BACKSLASH)) camera->zoom = 1.0f;
}
