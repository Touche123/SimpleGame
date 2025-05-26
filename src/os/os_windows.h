#pragma once

#include <windows.h>
#include <windowsx.h>

#include "../core.h"
#include "os.h"

typedef struct FrameData {
    f64 delta_time;
    f64 last_frame;
    LARGE_INTEGER frequency;
} FrameData;

bool os_make_directory(const char* path);
bool os_file_read_all(const char* path, void** data_out, uptr* size_out);
bool os_file_exists(const char* path);
bool os_file_write_all(const char* path, const void* data, uptr size);

void os_init(void);
// OsWindowId os_window_open(u32 width, u32 height);
bool os_events_iter_next(OsEvent* e);
double os_windows_get_delta_time(void);
double os_windows_update_delta_time(void);
OsKey os_key_from_win32(WPARAM vk, LPARAM lparam);
int game_main(int arc, char** argv);
