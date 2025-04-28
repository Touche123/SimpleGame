#include "gpu_backend_vulkan.h"
#include "core.h"

VkInstance instance = {0};

int gpu_backend_vulkan_init() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    printf("%i extensions supported\n", extensionCount);
    return 1;
}
