#include <windows.h>
#include "../include/windows_utils.h"

bool GetExecutablePath(char *dest, size_t size)
{
  return GetModuleFileName(NULL, dest, size) == 0 ? false : true;
}
