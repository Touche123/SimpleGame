#include "../core.h"

typedef struct {
    const uint8_t* buffer;
    size_t size;
    size_t position;
} BinaryStreamReader;

void io_binary_stream_reader_init(BinaryStreamReader* reader, const uint8_t* buffer, size_t size) {
    reader->buffer = buffer;
    reader->size = size;
    reader->position = 0;
}

bool io_binary_stream_reader_read_u32(BinaryStreamReader* reader, uint32_t* out) {
    if (reader->position + sizeof(uint32_t) > reader->size) return false;
    memcpy(out, reader->buffer + reader->position, sizeof(uint32_t));
    reader->position += sizeof(uint32_t);
    return true;
}

bool io_binary_stream_reader_read_bytes(BinaryStreamReader* reader, void* out, size_t size) {
    if (reader->position + size > reader->size) {
        return false;  // out of bounds
    }

    memcpy(out, reader->buffer + reader->position, size);
    reader->position += size;
    return true;
}

bool io_binary_stream_reader_load_from_file(BinaryStreamReader* reader, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    uint8_t* buffer = malloc(size);
    if (!buffer) {
        fclose(f);
        return false;
    }

    fread(buffer, 1, size, f);
    fclose(f);

    io_binary_stream_reader_init(reader, buffer, size);
    return true;
}

void io_binary_stream_reader_free(BinaryStreamReader* reader) {
    free(reader->buffer);
    reader->buffer = NULL;
    reader->size = 0;
    reader->position = 0;
}