#include "core.h"

#include <stdio.h>

#define B_STACKTRACE_IMPL
#include <b_stacktrace.h>

_Noreturn uintptr_t core_abort(const char* file, int line, const char* message, ...) {
    fprintf(stderr, "abort: ");

    va_list va_args;
    va_start(va_args, message);
    vfprintf(stderr, message, va_args);
    va_end(va_args);

    fprintf(stderr, "\nfile: %s:%u\n", file, line);
    char* stacktrace = b_stacktrace_get_string();
    fprintf(stderr, "stacktrace:\n%s\n\n", stacktrace);
    abort();

}

void core_assert_failed(const char* cond, const char* file, int line, const char* message, ...) {
    fprintf(stderr, "assertion failed: %s\nmessage: ", cond);

    va_list va_args;
    va_start(va_args, message);
    vfprintf(stderr, message, va_args);
    va_end(va_args);

    fprintf(stderr, "\nfile: %s:%u\n", file, line);
    char* stacktrace = b_stacktrace_get_string();
    fprintf(stderr, "stacktrace:\n%s\n\n", stacktrace);
    abort();

}
