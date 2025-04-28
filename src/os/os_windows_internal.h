#pragma once

#include <windows.h>

#include "os.h"

typedef struct OsWindow OsWindow;
struct OsWindow {
    HWND hwnd;
    u32 width;
    u32 height;
};

typedef struct OsWindows OsWindows;
struct OsWindows {
    HINSTANCE hinstance;
    OsWindow window;
    OsEvent* e_out;
    bool first_mouse;
    int last_mouse_x;
    int last_mouse_y;
};

extern OsWindows g_os_windows;
bool os_mouse_enabled;
int game_main(int arc, char** argv);
void os_mouse_lock_to_center(HWND hwnd);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
