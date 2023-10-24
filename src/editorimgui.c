#include "../include/raylib.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../include/cimgui.h"
#define NO_FONT_AWESOME
#include "../include/rlImGui.h"
#include <stdbool.h>
#include <stdlib.h>

bool TileSelector();

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

        // D for demo window (shows all the things ImGui can do)
        static bool showDemo = false;
        if (IsKeyPressed(KEY_D)) showDemo = !showDemo;
        if (showDemo) igShowDemoWindow(NULL);

        // Function for prototype tile selector
        TileSelector();

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
