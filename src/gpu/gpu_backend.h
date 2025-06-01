// gpu_backend.h
#ifndef GPU_BACKEND_H
#define GPU_BACKEND_H

#include "../mesh.h"
#include "../os/os_windows.h"
#include "../shader.h"

typedef int (*gpu_init_fn)(int width, int height);
typedef void (*gpu_prepare_frame_fn)(void);
typedef void (*gpu_render_mesh_fn)(Mesh* mesh);
typedef void (*gpu_end_frame_fn)(void);
typedef void (*gpu_set_fbsize_fn)(int width, int height);
typedef void (*gpu_generate_brdf_fn)(Shader* shader);

typedef struct GpuBackend {
    gpu_init_fn init;
    gpu_prepare_frame_fn prepare_frame;
    gpu_render_mesh_fn render_mesh;
    gpu_end_frame_fn end_frame;
    gpu_set_fbsize_fn set_framebuffer_size;
    gpu_generate_brdf_fn generate_brdf_lut;
} GpuBackend;

// En “global” instans som vi kommer binda i main beroende på define:
extern GpuBackend gpu;

#endif  // GPU_BACKEND_H