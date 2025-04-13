#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "defines.h"
#include "shader.h"
#include "model.h"

typedef struct Texture{
} Texture;

void asset_manager_init(void);
void asset_manager_shutdown(void);

Material* asset_get_material(const char* name);
Shader* asset_get_shader(const char* name);

void asset_poll_watchers(void);
#endif
