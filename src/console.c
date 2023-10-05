#include <stdlib.h>
#include <stdalign.h>
#include <string.h>
#include <stdio.h>
#include "../include/console.h"

// TODO
// global offset instead of tracking position of EVERY message
// first and less message proper bounding
// OVERWRITE when more messages written than can be held!

#define BUTTON_SCROLL 10
#define SCROLLWHEEL_MULT 5

#define X_OFFSET 5

bool InitConsole(Console *log, Arena *arena, Rectangle rect, uint32_t maxMessages, uint32_t charLimit, uint32_t numRows, Color background, Color font)
{
    log->enabled = true;
    log->rect = rect;
    log->outline = (Rectangle){0, 0, 0, 0};
    log->outlineThick = 0;
    log->outlineColor = BLACK;

    log->maxMessages = maxMessages;
    log->charLimit = charLimit;
    log->numMessages = 0;
    log->numRows = numRows;
    log->globalOffset = 0;

    log->displayStart = 0;
    log->displayEnd = 0;

    log->messages = PushArray(arena, ConsoleMessage, maxMessages);
    for (int i = 0; i < maxMessages; i++)
    {
        log->messages[i].message = PushArray(arena, char, charLimit);
    }

    log->scrollUp = -1;
    log->scrollDown = -1;
    log->toggle = -1;

    log->backgroundColor = background;
    log->fontColor = font;
    log->fontSize = (rect.height) / (float)numRows;

    return true;
}

void ConsoleSetKeys(Console *log, uint16_t up, uint16_t down, uint16_t toggle)
{
    log->scrollUp = up;
    log->scrollDown = down;
    log->toggle = toggle;
}

void ConsoleSetOutline(Console *log, float x, float y, float width, float height, float thickness, Color color)
{
    log->outline = (Rectangle){ x, y, width, height };
    log->outlineThick = thickness;
    log->outlineColor = color;
}

void WriteConsole(Console *log, char *message)
{
    // Copy the message passed to the nearest empty message memory,
    // this is done to avoid the user passing a string with limited scope
    strcpy(log->messages[log->numMessages].message, message);

    // If this message would over-fill the window, scroll the rest of the text up by one font height,
    // this currently needs fixing as it will not recognize if you scroll the text up manually
    if (log->numMessages >= log->numRows)
    {
        log->globalOffset += log->fontSize;
    }

    // If this is NOT the first message, place the text relative to the last message down by one font height,
    // and increment displayEnd for sliding window
    if (log->numMessages > 0)
    {
        log->messages[log->numMessages].localOffset = log->messages[log->numMessages - 1].localOffset + log->fontSize; 
        log->displayEnd++;
        log->numMessages++;
        return;
    }

    // If this IS the first message, place it at the uppsermost corner of the console
    // and set the sliding window to only display this message
    log->messages[log->numMessages].localOffset = log->globalOffset;
    log->displayEnd = 0;
    log->displayStart = 0;
    log->numMessages++;
}

void ConsoleUpdate(Console *log)
{
    // Toggle visibility/updating of the console
    if (IsKeyPressed(log->toggle))
    {
        log->enabled = !log->enabled;
    }

    if (!log->enabled) return;

    DrawRectangleRec(log->rect, log->backgroundColor);

    if (log->numMessages == 0) return;

    // Variables for ease of reading
    const float firstMsgOffset = log->messages[0].localOffset;
    const float endDisplayOffset = log->messages[log->displayEnd].localOffset;
    const Rectangle consoleRect = log->rect;

    // If the LAST message being rendered would go beyond the upper bound, do not move,
    // otherwise scroll text up
    if (IsKeyPressed(log->scrollUp) && endDisplayOffset + log->globalOffset > consoleRect.y)
    {
        log->globalOffset -= BUTTON_SCROLL;
    }

    // If the first message in the list would go beyond the first "row", do not move,
    // otherwise scroll text down
    if (IsKeyPressed(log->scrollDown) && firstMsgOffset + log->globalOffset < consoleRect.y)
    {
        log->globalOffset += BUTTON_SCROLL;
    }

    // Move all messages according to how much the mouse wheel moved
    float mouseWheel = GetMouseWheelMove();
    if ((mouseWheel < 0 && endDisplayOffset + log->globalOffset > consoleRect.y) || (mouseWheel > 0 && firstMsgOffset + log->globalOffset < consoleRect.y))
    {
        log->globalOffset += mouseWheel * SCROLLWHEEL_MULT;
    }

    // If any text is one font height above the console bounds, stop it from rendering.
    // This is intended to be used with some sort of border to mask the text going beyond the console bounds.
    float topCutoff = log->rect.y - log->fontSize;
    float bottomCutoff = log->rect.y + log->rect.height;
    for (int i = log->displayStart; i <= log->displayEnd; i++)
    {
        float totalOffset = log->messages[i].localOffset + log->globalOffset;
        if (totalOffset < topCutoff)
        {
            log->displayStart++;
        }

        if (totalOffset > bottomCutoff)
        {
            log->displayEnd--;
        }
    }

    // If any unrendered text comes back within the rendered window, re-render it,
    // again, this is intended to be used with some sort of border to mask the text going beyond the console bounds.
    for (int i = 0; i < log->displayStart; i++)
    {
        float totalOffset = log->messages[i].localOffset + log->globalOffset;
        if (totalOffset > topCutoff)
        {
            log->displayStart--;
        }
    }

    for (int i = log->numMessages - 1; i > log->displayEnd; i--)
    {
        float totalOffset = log->messages[i].localOffset + log->globalOffset;
        if (totalOffset < bottomCutoff)
        {
            log->displayEnd++;
        }
    }

    // Bound first and less messages to prevent them from clipping past
    // where they should be
    if (log->messages[0].localOffset + log->globalOffset > log->rect.y)
    {
        float difference = (log->messages[0].localOffset + log->globalOffset) - log->rect.y;
        log->globalOffset -= difference;
    }

    if (log->messages[log->numMessages - 1].localOffset + log->globalOffset < log->rect.y)
    {
        float difference = log->rect.y - (log->messages[log->numMessages - 1].localOffset + log->globalOffset);
        log->globalOffset += difference;
    }

    // Render any text within the rendered sliding window
    for (int i = log->displayStart; i <= log->displayEnd; i++)
    {
        float yPos = log->messages[i].localOffset + log->globalOffset;
        DrawText(log->messages[i].message, X_OFFSET, yPos, log->fontSize, log->fontColor);
    }

    if (log->outlineThick > 0)
    {
        DrawRectangleLinesEx(log->outline, log->outlineThick, log->outlineColor);
    }
}
