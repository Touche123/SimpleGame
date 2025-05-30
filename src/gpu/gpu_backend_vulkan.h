#ifndef GPU_BACKEND_VULKAN_H
#define GPU_BACKEND_VULKAN_H

#include <vulkan/vulkan.h>

#include "../mesh.h"
#include "../os/os_windows.h"
#include "../shader.h"

int gpu_backend_vulkan_init(int width, int height, OsWindowBackend window);

// Förbered ram: börja command buffer och render pass
void gpu_backend_vulkan_prepare_frame();

// Rendera ett mesh via inspelade command buffers
void gpu_backend_vulkan_render_mesh(Mesh* mesh);

// Avsluta ram: avsluta render pass, submit och present
void gpu_backend_vulkan_end_frame();

// Hantera fönsterstorleksändring
void gpu_backend_vulkan_set_framebuffer_size(int width, int height);

// Generera BRDF LUT som Vulkan-textur, returnerar VkImageView
VkImageView gpu_backend_vulkan_generate_brdf_lut(Shader* shader);
#endif
