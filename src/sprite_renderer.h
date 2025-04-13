#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

static unsigned int VBO, VAO, EBO;
static unsigned int shaderProgram;

void sprite_renderer_init();
void sprite_renderer_draw();
void sprite_renderer_destroy();

#endif