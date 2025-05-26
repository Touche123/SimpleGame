#include "shader.h"

#include <stdio.h>

#include "glad.h"
#include "os/os_windows.h"

char* shader_read_file(const char* filename, size_t* size_out) {
    void* data = NULL;
    uptr size = 0;
    if (os_file_read_all(filename, &data, &size)) {
        if (size_out) *size_out = (size_t)size;
        return (char*)data;
    }

    return NULL;
}

unsigned int shader_compile(const char* vertex_filename, const char* fragment_filename) {
    const char* vertex_src = shader_read_file(vertex_filename, NULL);
    const char* fragment_src = shader_read_file(fragment_filename, NULL);

    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_src, NULL);
    // free((void*)vertex_src);
    glCompileShader(vertex_shader);
    // check for shader compile errors
    s32 success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // fragment shader
    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_src, NULL);
    // free((void*)fragment_src);
    glCompileShader(fragment_shader);
    // check for shader compile errors
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }
    // link shaders
    u32 shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    // check for linking errors
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    free((void*)vertex_src);
    free((void*)fragment_src);

    return shader_program;
}

void shader_recompile(Shader* shader) {
    printf("Shader Recompile: %s, %s", shader->vertex_shader_filename, shader->fragment_shader_filename);
    unsigned int reloaded_program =
        shader_compile(shader->vertex_shader_filename, shader->fragment_shader_filename);
    if (reloaded_program) {
        glDeleteProgram(shader->id);
        shader->id = reloaded_program;
    }
}

void shader_uniform1i(Shader* shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}

void shader_uniform1f(Shader* shader, const char* name, float value) {
    glUniform1f(glGetUniformLocation(shader->id, name), value);
}
void shader_uniform2f(Shader* shader, const char* name, float value) {
    glUniform2f(glGetUniformLocation(shader->id, name), value, value);
}
void shader_uniform3f(Shader* shader, const char* name, float value) {
    glUniform3f(glGetUniformLocation(shader->id, name), value, value, value);
}
void shader_uniform3fv(Shader* shader, const char* name, const float* value) {
    glUniform3fv(glGetUniformLocation(shader->id, name), 1, value);
}
void shader_uniformMatrix4fv(Shader* shader, const char* name, const float* value) {
    glUniformMatrix4fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, value);
}
void shader_use(Shader* shader) {
    glUseProgram(shader->id);
}
