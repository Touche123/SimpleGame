#ifndef SHADER_H
#define SHADER_H
#include "defines.h"

typedef struct Shader {
    unsigned int id;
    const char* vertex_shader_filename;
    const char* fragment_shader_filename;
} Shader;

unsigned int shader_compile(const char* vertex_source, const char* fragment_source);
char* shader_read_file(const char* filename);
void shader_recompile(Shader* shader);
#endif
