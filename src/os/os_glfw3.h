#pragma once

#include "os.h"

void os_init(void);

OsWindowId os_window_open(u32 width, u32 height);

bool os_events_iter_next(OsEvent* e);
