#include "../core.h"

typedef struct {
    uint8_t* buffer;
    size_t capacity;
    size_t position;
} BinaryStreamWriter;

bool io_binary_streamwriter_init(BinaryStreamWriter* writer, size_t initial_capacity);
void io_binary_streamwriter_free(BinaryStreamWriter* writer);
bool io_binary_streamwriter_ensure_capacity(BinaryStreamWriter* writer, size_t extra);
bool io_binary_streamwriter_write_u32(BinaryStreamWriter* writer, uint32_t value);
bool io_binary_streamwriter_write_bytes(BinaryStreamWriter* writer, const void* data, size_t size);
void io_binary_streamwriter_save_to_file(BinaryStreamWriter* writer, const char* path);