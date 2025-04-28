#pragma once

#include "os.h"

#include <glfw3.h>

typedef struct OsGlfw3 OsGlfw3;
struct OsGlfw3 {
    GLFWwindow* window;
};

extern OsGlfw3 g_os_glfw3;
extern OsEvent last_key_event;
