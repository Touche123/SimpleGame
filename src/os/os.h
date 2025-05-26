#pragma once

#include "../core.h"

typedef enum OsKey {
    OS_KEY_UNKNOWN = 0,

    OS_KEY_A,
    OS_KEY_B,
    OS_KEY_C,
    OS_KEY_D,
    OS_KEY_E,
    OS_KEY_F,
    OS_KEY_G,
    OS_KEY_H,
    OS_KEY_I,
    OS_KEY_J,
    OS_KEY_K,
    OS_KEY_L,
    OS_KEY_M,
    OS_KEY_N,
    OS_KEY_O,
    OS_KEY_P,
    OS_KEY_Q,
    OS_KEY_R,
    OS_KEY_S,
    OS_KEY_T,
    OS_KEY_U,
    OS_KEY_V,
    OS_KEY_W,
    OS_KEY_X,
    OS_KEY_Y,
    OS_KEY_Z,

    OS_KEY_0,
    OS_KEY_1,
    OS_KEY_2,
    OS_KEY_3,
    OS_KEY_4,
    OS_KEY_5,
    OS_KEY_6,
    OS_KEY_7,
    OS_KEY_8,
    OS_KEY_9,

    OS_KEY_ESCAPE,
    OS_KEY_SPACE,
    OS_KEY_ENTER,
    OS_KEY_TAB,
    OS_KEY_BACKSPACE,
    OS_KEY_LEFT,
    OS_KEY_RIGHT,
    OS_KEY_UP,
    OS_KEY_DOWN,

    OS_KEY_LSHIFT,
    OS_KEY_RSHIFT,
    OS_KEY_LCTRL,
    OS_KEY_RCTRL,
    OS_KEY_LALT,
    OS_KEY_RALT,

    OS_KEY_F1,
    OS_KEY_F2,
    OS_KEY_F3,
    OS_KEY_F4,
    OS_KEY_F5,
    OS_KEY_F6,
    OS_KEY_F7,
    OS_KEY_F8,
    OS_KEY_F9,
    OS_KEY_F10,
    OS_KEY_F11,
    OS_KEY_F12,

    OS_KEY_COUNT  // Bra för loopar eller arrayer
} OsKey;

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
        OsKey key;
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
