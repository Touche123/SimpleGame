#pragma once

#include "../core.h"

typedef int OsEventType;
enum OsEventType {
    OS_EVENT_TYPE_UNKNOWN,
    OS_EVENT_TYPE_KEY_PRESSED,
    OS_EVENT_TYPE_KEY_RELEASED,
    OS_EVENT_TYPE_KEY_TYPED,
    OS_EVENT_TYPE_MOUSE_MOVED,
    OS_EVENT_TYPE_WINDOW_RESIZED,
    OS_EVENT_TYPE_WINDOW_CLOSED,
};

CORE_ID_DEFINE(OsWindowId);

typedef struct OsEvent OsEvent;
struct OsEvent {
    OsEventType type;
    OsWindowId window_id;
    union {
        char key;
        struct {
            u32 window_width;
            u32 window_height;
        } window_size;
        struct {
            s32 mouse_x;  // Absolut position
            s32 mouse_y;
            s32 mouse_delta_x;  // Delta (offset)
            s32 mouse_delta_y;
        } mouse_move;
    };
};

typedef struct OsWindowBackend OsWindowBackend;
struct OsWindowBackend {
    void* connection;
    void* window;
};

void os_mouse_show(bool show);
void os_init(void);
OsWindowId os_window_open(u32 width, u32 height);
OsWindowBackend os_window_backend(OsWindowId window_id);
bool os_events_iter_next(OsEvent* e);
