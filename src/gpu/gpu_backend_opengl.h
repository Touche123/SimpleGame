#ifndef GPU_BACKEND_OPENGL_H
#define GPU_BACKEND_OPENGL_H

#include "../glad.h"
#include "../os/os_windows.h"

struct Mesh;

int gpu_backend_opengl_init(int width, int height);
void gpu_backend_opengl_render_mesh(struct Mesh *mesh);
void gpu_backend_opengl_prepare_frame();
void gpu_backend_opengl_end_frame();
void gpu_backend_opengl_renderFullscreenQuad();
void gpu_backend_opengl_set_framebuffer_size(int width, int height);
#endif
