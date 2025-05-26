#include "../core.h"

typedef struct {
    uint8_t* buffer;
    size_t capacity;
    size_t position;
} BinaryStreamWriter;

bool io_binary_streamwriter_init(BinaryStreamWriter* writer, size_t initial_capacity) {
    writer->buffer = malloc(initial_capacity);
    writer->capacity = initial_capacity;
    writer->position = 0;
    return writer->buffer != NULL;
}

void io_binary_streamwriter_free(BinaryStreamWriter* writer) {
    free(writer->buffer);
    writer->buffer = NULL;
    writer->capacity = 0;
    writer->position = 0;
}

bool io_binary_streamwriter_ensure_capacity(BinaryStreamWriter* writer, size_t extra) {
    size_t needed = writer->position + extra;
    if (needed > writer->capacity) {
        size_t new_capacity = writer->capacity * 2;
        while (new_capacity < needed) new_capacity *= 2;
        void* new_buffer = realloc(writer->buffer, new_capacity);
        if (!new_buffer) return false;
        writer->buffer = new_buffer;
        writer->capacity = new_capacity;
    }
    return true;
}

bool io_binary_streamwriter_write_u32(BinaryStreamWriter* writer, uint32_t value) {
    if (!io_binary_streamwriter_ensure_capacity(writer, sizeof(value))) return false;
    memcpy(writer->buffer + writer->position, &value, sizeof(value));
    writer->position += sizeof(value);
    return true;
}

bool io_binary_streamwriter_write_bytes(BinaryStreamWriter* writer, const void* data, size_t size) {
    if (!io_binary_streamwriter_ensure_capacity(writer, size)) {
        return false;
    }

    memcpy(writer->buffer + writer->position, data, size);
    writer->position += size;
    return true;
}

void io_binary_streamwriter_save_to_file(BinaryStreamWriter* writer, const char* path) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        printf("Failed to open file for writing: %s\n", path);
        return;
    }

    fwrite(writer->buffer, 1, writer->position, f);
    fclose(f);
}
