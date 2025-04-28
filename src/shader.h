#ifndef SHADER_H
#define SHADER_H
#include "core.h"

typedef struct Shader {
    unsigned int id;
    const char* vertex_shader_filename;
    const char* fragment_shader_filename;
} Shader;

unsigned int shader_compile(const char* vertex_source, const char* fragment_source);
char* shader_read_file(const char* filename);
void shader_recompile(Shader* shader);
void shader_use(Shader* shader);
void shader_uniform1i(Shader* shader, const char* name, int value);
void shader_uniform1f(Shader* shader, const char* name, float value);
void shader_uniform2f(Shader* shader, const char* name, float value);
void shader_uniform3f(Shader* shader, const char* name, float value);
void shader_uniform3fv(Shader* shader, const char* name, const float* value);
void shader_uniformMatrix4fv(Shader* shader, const char* name, const float* value);
#endif
