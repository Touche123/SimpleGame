#include "os_glfw3_internal.h"


void glfw_char_callback(GLFWwindow* window, unsigned int codepoint);
void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void os_init(void) {
    CORE_ASSERT(glfwInit(), "Could not initialize glfw");
}

OsWindowId os_window_open(u32 width, u32 height) {
    static int times_called = 0;
    CORE_ASSERT(times_called == 0, "multiple windows is unsupported at this time");
    times_called += 1;

    OsWindowId id = { 1 };

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_os_glfw3.window = glfwCreateWindow(width, height, "Hello world", NULL, NULL);

    if (!g_os_glfw3.window) {
        glfwTerminate();
        CORE_ABORT("Could not create glfw3 window");
    }

    glfwSetCharCallback(g_os_glfw3.window, glfw_char_callback);
    glfwSetKeyCallback(g_os_glfw3.window, glfw_key_callback);
    glfwMakeContextCurrent(g_os_glfw3.window);

    return id;
}

bool os_events_iter_next(OsEvent* e_out) {
    glfwPollEvents();

    if (last_key_event.type != OS_EVENT_TYPE_UNKNOWN) {
        *e_out = last_key_event;
        last_key_event.type = OS_EVENT_TYPE_UNKNOWN; // Reset after use
        return true;
    }

    return false;
}

void glfw_char_callback(GLFWwindow* window, unsigned int codepoint) {
    //printf("char callback\n");
    //last_key_event.type = OS_EVENT_TYPE_KEY_PRESSED;
    //last_key_event.key = codepoint; // ASCII value or Unicode
}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) {
        last_key_event.type = OS_EVENT_TYPE_KEY_RELEASED;
        last_key_event.key = key; // this is the GLFW_KEY_* constant (not ASCII!)
    }
    if (action == GLFW_PRESS) {
        last_key_event.type = OS_EVENT_TYPE_KEY_PRESSED;
        last_key_event.key = key; // this is the GLFW_KEY_* constant (not ASCII!)
    }
}

OsGlfw3 g_os_glfw3;
OsEvent last_key_event = { OS_EVENT_TYPE_UNKNOWN, 0 };
