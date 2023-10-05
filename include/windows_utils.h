#include <stdlib.h>
#include <stdbool.h>

// This file exists to avoid conflicts when compiling with raylib.
// windows.h and raylib/raygui do not play well when included in the same file.

bool GetExecutablePath(char *dest, size_t size);
