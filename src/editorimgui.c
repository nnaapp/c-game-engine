#include "../include/raylib.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../include/cimgui.h"
#define NO_FONT_AWESOME
#include "../include/rlImGui.h"
#include <stdbool.h>
#include <stdlib.h>

int main(void)
{
    Vector2 displaySize = { 1024, 1024 };
    
    // Raylib window initialization
    InitWindow(displaySize.x, displaySize.y, "ImGui Test");
    SetTargetFPS(144);

    // cImGui and rlImGui setup
    rlImGuiSetup(true);

    while (!WindowShouldClose())
    {        
        // Begin raylib frame
        BeginDrawing();

        ClearBackground(DARKGRAY);

        // cImGui and rlImGui start frame
        rlImGuiBegin();

        igShowDemoWindow(NULL);
        // bool open = true;
        // igShowDemoWindow(&open);
        // igText("Hello world!");

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
