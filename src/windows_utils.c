#include <libloaderapi.h>
#include <sysinfoapi.h>
#include "../include/windows_utils.h"

bool GetExecutablePath(char *dest, size_t size)
{
  return GetModuleFileName(NULL, dest, size) == 0 ? false : true;
}

int32_t GetPageSize()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}
