#include "os_windows.h"

#include "os_windows_internal.h"

bool os_make_directory(const char* path) {
    if (CreateDirectory(path, NULL) == 0) {
        return false;
    }
    return true;
}

bool os_file_read_all(const char* path, void** data_out, uptr* size_out) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return false;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    void* data = malloc(file_size + 1);
    if (!data) {
        fclose(f);
        return false;
    }

    long read_size = fread(data, 1, file_size, f);
    fclose(f);

    if (read_size != file_size) {
        free(data);
        return false;
    }

    ((char*)data)[file_size] = '\0';

    *data_out = data;
    *size_out = file_size;
    return true;
}

bool os_file_write_all(const char* path, const void* data, uptr size) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        return false;
    }

    uptr write_size = fwrite(data, 1, size, f);
    fclose(f);
    return write_size == size;
}

bool os_file_exists(const char* path) {
    u32 w = GetFileAttributesA(path);
    if (w == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    return !(w & FILE_ATTRIBUTE_DIRECTORY);
}

double GetTimeSeconds(LARGE_INTEGER frequency) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart / (double)frequency.QuadPart;
}

OsWindows g_os_windows;
FrameData framedata = {0};

int game_main(int arc, char** argv);

void os_init(void) {
    g_os_windows.hinstance = GetModuleHandle(NULL);
    timeBeginPeriod(1);
    SetProcessAffinityMask(GetCurrentProcess(), 1);
    QueryPerformanceFrequency(&framedata.frequency);

    if (framedata.frequency.QuadPart == 0) {
        printf("High-resolution performance counter not supported.\n");
    }
    framedata.last_frame = GetTimeSeconds(framedata.frequency);
}

double os_windows_update_delta_time(void) {
    double current_time = GetTimeSeconds(framedata.frequency);
    framedata.delta_time = current_time - framedata.last_frame;
    framedata.last_frame = current_time;
}

double os_windows_get_delta_time(void) {
    return framedata.delta_time;
    // fps_timer += g_os_windows_frame_data.delta_time;
    // frame_count++;

    // f32 current_frame = glfwGetTime();
    // frame_data.delta_time = current_frame - frame_data.last_frame;
    // frame_data.last_frame = current_frame;
    // if (fps_timer >= 1.0) {
    //     fps = (double)frame_count / fps_timer;
    //     printf("FPS: %.2f\n", fps);
    //     fps_timer = 0.0;
    //     frame_count = 0;
    // }
    //        printf("delta_time: %f\n", frame_data.delta_time);
}

OsWindowId os_window_open(u32 width, u32 height) {
    static int times_called = 0;
    CORE_ASSERT(times_called == 0, "multiple windows is unsupported at this time");
    times_called += 1;

    OsWindowId id = {1};

    const char CLASS_NAME[] = "Game Window Class";

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = g_os_windows.hinstance;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;

    ATOM class_atom = RegisterClassExA(&wc);
    CORE_ASSERT(class_atom != 0, "failed to register class: %lx", GetLastError());

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        CORE_GAME_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL,
        NULL,
        g_os_windows.hinstance,
        NULL);
    CORE_ASSERT(hwnd, "failed to open window: %lx", GetLastError());

    ShowWindow(hwnd, true);

    os_mouse_lock_to_center(hwnd);
    g_os_windows.window.hwnd = hwnd;
    g_os_windows.window.width = width;
    g_os_windows.window.height = height;

    return id;
}

void os_mouse_lock_to_center(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);

    POINT center;
    center.x = (rect.right - rect.left) / 2;
    center.y = (rect.bottom - rect.top) / 2;
    ClientToScreen(hwnd, &center);
    os_mouse_show(false);
    SetCursorPos(center.x, center.y);
}

OsWindowBackend os_window_backend(OsWindowId window_id) {
    CORE_UNUSED(window_id);

    OsWindowBackend backend = {
        .connection = (void*)(uptr)g_os_windows.hinstance,
        .window = (void*)(uptr)g_os_windows.window.hwnd};

    return backend;
}

bool os_events_iter_next(OsEvent* e_out) {
    e_out->type = OS_EVENT_TYPE_UNKNOWN;
    g_os_windows.e_out = e_out;

    MSG msg;
    if (!PeekMessage(&msg, g_os_windows.window.hwnd, 0, 0, PM_REMOVE)) {
        return false;
    }

    TranslateMessage(&msg);
    DispatchMessage(&msg);

    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    CORE_UNUSED(hInstance);
    CORE_UNUSED(hPrevInstance);
    CORE_UNUSED(lpCmdLine);
    CORE_UNUSED(nShowCmd);

    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);

    // int argc;
    // argv = CommandLineToArgvW(LPCWSTR lpCmdLine, &argc);
    return game_main(0, NULL);
}

void os_mouse_show(bool show) {
    if (show) {
        os_mouse_enabled = true;
        ShowCursor(TRUE);
    } else {
        os_mouse_enabled = false;
        ShowCursor(FALSE);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    OsEvent* e = g_os_windows.e_out;

    if (e) {
        switch (msg) {
            case WM_SIZE:
                e->type = OS_EVENT_TYPE_WINDOW_RESIZED;
                e->window_size.window_width = LOWORD(lParam);
                e->window_size.window_height = HIWORD(lParam);
                g_os_windows.e_out = NULL;
                return 0;

            case WM_CHAR:
                e->type = OS_EVENT_TYPE_KEY_TYPED;
                e->key = wParam;
                g_os_windows.e_out = NULL;
                return 0;

            case WM_CLOSE:
                e->type = OS_EVENT_TYPE_WINDOW_CLOSED;
                g_os_windows.e_out = NULL;
                return 0;

            case WM_KEYDOWN:
                e->type = OS_EVENT_TYPE_KEY_PRESSED;
                e->key = wParam;
                g_os_windows.e_out = NULL;
                return 0;

            case WM_KEYUP:
                e->type = OS_EVENT_TYPE_KEY_RELEASED;
                e->key = wParam;
                g_os_windows.e_out = NULL;
                return 0;
            case WM_MOUSEMOVE:
                POINT center;
                RECT rect;
                GetClientRect(hwnd, &rect);
                center.x = (rect.right - rect.left) / 2;
                center.y = (rect.bottom - rect.top) / 2;

                int xpos = GET_X_LPARAM(lParam);
                int ypos = GET_Y_LPARAM(lParam);

                if (g_os_windows.first_mouse) {
                    g_os_windows.last_mouse_x = xpos;
                    g_os_windows.last_mouse_y = ypos;
                    g_os_windows.first_mouse = false;
                }

                int delta_x = xpos - center.x;
                int delta_y = center.y - ypos;  // Y uppåt i OpenGL

                g_os_windows.last_mouse_x = xpos;
                g_os_windows.last_mouse_y = ypos;

                e->type = OS_EVENT_TYPE_MOUSE_MOVED;
                e->mouse_move.mouse_x = xpos;
                e->mouse_move.mouse_y = ypos;
                e->mouse_move.mouse_delta_x = delta_x;
                e->mouse_move.mouse_delta_y = delta_y;
                g_os_windows.e_out = NULL;

                ClientToScreen(hwnd, &center);
                SetCursorPos(center.x, center.y);
                return 0;

            default:
                return DefWindowProcA(hwnd, msg, wParam, lParam);
        }
    } else {
        switch (msg) {
            default:
                return DefWindowProcA(hwnd, msg, wParam, lParam);
        }
    }
}
