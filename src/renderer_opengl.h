#ifndef RENDERER_OPENGL_H
#define RENDERER_OPENGL_H

struct Mesh;

int renderer_opengl_init(int width, int height);
void renderer_opengl_render_mesh(struct Mesh *mesh);
void renderer_opengl_prepare_frame();
#endif
