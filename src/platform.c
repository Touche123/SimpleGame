#include "platform.h"

void platform_find_file(char* path) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(path, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No files found.\n");
    } else {
        do {
            printf("Found file: %s\n", findFileData.cFileName);
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
}

HANDLE platform_register_watch_directory(char* path) {
    DWORD dwWaitStatus;
    HANDLE dwChangeHandle;
    TCHAR lpDrive[4];
    TCHAR lpFile[_MAX_FNAME];
    TCHAR lpExt[_MAX_EXT];

    dwChangeHandle = FindFirstChangeNotification(
            path,                           // directory to watch
            TRUE,                           // do not watch subtree
            FILE_NOTIFY_CHANGE_FILE_NAME | 
            FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_SIZE
            );

    if (dwChangeHandle == INVALID_HANDLE_VALUE) {
        printf("\n ERROR: FindFirstChangeNotification function failed.\n");
        ExitProcess(GetLastError());
    }

    return dwChangeHandle;
}

bool platform_watch_event(HANDLE handle) {
    DWORD wait_status = WaitForSingleObject(handle, 0);
    if (wait_status == WAIT_OBJECT_0) {
        if (!FindNextChangeNotification(handle)) {
            printf("Failed to rearm file watcher!\n");
        }
        return true;
    }
    return false;
}
