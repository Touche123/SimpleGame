#include "../core.h"

typedef struct {
    const uint8_t* buffer;
    size_t size;
    size_t position;
} BinaryStreamReader;

void io_binary_stream_reader_init(BinaryStreamReader* reader, const uint8_t* buffer, size_t size);
bool io_binary_stream_reader_read_u32(BinaryStreamReader* reader, uint32_t* out);
bool io_binary_stream_reader_read_bytes(BinaryStreamReader* reader, void* out, size_t size);
bool io_binary_stream_reader_load_from_file(BinaryStreamReader* reader, const char* path);
void io_binary_stream_reader_free(BinaryStreamReader* reader);