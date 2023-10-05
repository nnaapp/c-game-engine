#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include "../include/raylib.h"
#include "../include/arena.h"

typedef struct ConsoleMessage
{
    char *message;
    float localOffset;
} ConsoleMessage;

typedef struct Console
{
    bool enabled;
    Rectangle rect;
    Rectangle outline;
    float outlineThick;
    Color outlineColor;

    ConsoleMessage *messages;
    uint32_t numMessages;
    uint32_t maxMessages;
    uint32_t charLimit;
    uint32_t numRows;
    float globalOffset;

    uint32_t displayStart;
    uint32_t displayEnd;

    uint16_t scrollUp;
    uint16_t scrollDown;
    uint16_t toggle;

    Color backgroundColor;
    Color fontColor;
    uint32_t fontSize;
} Console;

bool InitConsole(Console *log, Arena *arena, Rectangle rect, uint32_t maxMessages, uint32_t charLimit, uint32_t numRows, Color background, Color font);

void ConsoleSetKeys(Console *log, uint16_t up, uint16_t down, uint16_t toggle);

void ConsoleSetOutline(Console *log, float x, float y, float width, float height, float thickness, Color color);

void WriteConsole(Console *log, char *message);

void ConsoleUpdate(Console *log);

#endif
