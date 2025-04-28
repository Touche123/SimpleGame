#ifndef PLATORM_H
#define PLATORM_H

#include <windows.h>
#include "core.h"

void platform_find_file(char* path);
HANDLE platform_register_watch_directory(char* path);
bool platform_watch_event(HANDLE handle);
#endif
