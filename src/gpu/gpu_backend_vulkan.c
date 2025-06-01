// gpu_backend_vulkan.h
#ifndef GPU_BACKEND_VULKAN_H
#define GPU_BACKEND_VULKAN_H

#include <vulkan/vulkan.h>

#include "../mesh.h"
#include "../shader.h"
#define VK_USE_PLATFORM_WIN32_KHR
// Initierar Vulkan, skapar instans, fysisk/logisk enhet, swapchain, pipelines etc.
// Returnerar 1 vid framgång, 0 vid fel.
int gpu_backend_vulkan_init(int width, int height);

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

#endif  // GPU_BACKEND_VULKAN_H

// gpu_backend_vulkan.c
#define VK_USE_PLATFORM_WIN32_KHR
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>


#include "gpu_backend_vulkan.h"
// #include "os/os_windows.h"  // eller passande OS wrapper

// Struktur för att hålla all Vulkan state
typedef struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    uint32_t swapchainImageCount;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer* framebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
} VulkanContext;

static VulkanContext vkCtx;

// Framskrivna interna funktioner (implementera enligt VK-Tutorial)
static void initVulkan(int width, int height);
static void cleanupSwapchain();
static void recreateSwapchain(int width, int height);

// ------------ Hjälpfunktioner ------------

// Kontrollera fel
static void checkVkResult(VkResult res, const char* msg) {
    if (res != VK_SUCCESS) {
        fprintf(stderr, "Vulkan error: %s (code %d)\n", msg, res);
        abort();
    }
}

// Välj fysisk enhet
static void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkCtx.instance, &deviceCount, NULL);
    assert(deviceCount > 0 && "No Vulkan-compatible GPU found");
    VkPhysicalDevice devices[16];
    vkEnumeratePhysicalDevices(vkCtx.instance, &deviceCount, devices);
    // Här väljer vi bara första med stöd för nödvändiga features
    for (uint32_t i = 0; i < deviceCount; i++) {
        vkCtx.physicalDevice = devices[i];
        break;
    }
}

// Hitta queue-familjer för grafik och presentation
typedef struct QueueFamilyIndices {
    int graphicsFamily;
    int presentFamily;
} QueueFamilyIndices;

static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = {-1, -1};
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties families[16];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, families);
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.graphicsFamily >= 0 && indices.presentFamily >= 0) {
            break;
        }
    }
    return indices;
}

// Skapa logisk enhet och hämta queues
static void createLogicalDevice(VkSurfaceKHR surface) {
    QueueFamilyIndices indices = findQueueFamilies(vkCtx.physicalDevice, surface);
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[2];
    uint32_t queueCreateInfoCount = 0;

    if (indices.graphicsFamily == indices.presentFamily) {
        queueCreateInfos[0] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = indices.graphicsFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority};
        queueCreateInfoCount = 1;
    } else {
        queueCreateInfos[0] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = indices.graphicsFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority};
        queueCreateInfos[1] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = indices.presentFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority};
        queueCreateInfoCount = 2;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {0};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.depthClamp = VK_TRUE;

    VkDeviceCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.pEnabledFeatures = &deviceFeatures;
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    checkVkResult(vkCreateDevice(vkCtx.physicalDevice, &createInfo, NULL, &vkCtx.device),
                  "Failed to create logical device");

    vkGetDeviceQueue(vkCtx.device, indices.graphicsFamily, 0, &vkCtx.graphicsQueue);
    vkGetDeviceQueue(vkCtx.device, indices.presentFamily, 0, &vkCtx.presentQueue);
}

// Hitta bästa format
static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, uint32_t formatCount) {
    for (uint32_t i = 0; i < formatCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, uint32_t modeCount) {
    for (uint32_t i = 0; i < modeCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities, int width, int height) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    } else {
        VkExtent2D actualExtent = {(uint32_t)width, (uint32_t)height};
        actualExtent.width = (actualExtent.width < capabilities->minImageExtent.width) ? capabilities->minImageExtent.width : actualExtent.width;
        actualExtent.width = (actualExtent.width > capabilities->maxImageExtent.width) ? capabilities->maxImageExtent.width : actualExtent.width;
        actualExtent.height = (actualExtent.height < capabilities->minImageExtent.height) ? capabilities->minImageExtent.height : actualExtent.height;
        actualExtent.height = (actualExtent.height > capabilities->maxImageExtent.height) ? capabilities->maxImageExtent.height : actualExtent.height;
        return actualExtent;
    }
}

// Skapa swapchain och dess image views
static void createSwapchain(int width, int height) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkCtx.physicalDevice, vkCtx.surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkCtx.physicalDevice, vkCtx.surface, &formatCount, NULL);
    VkSurfaceFormatKHR formats[16];
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkCtx.physicalDevice, vkCtx.surface, &formatCount, formats);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats, formatCount);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkCtx.physicalDevice, vkCtx.surface, &presentModeCount, NULL);
    VkPresentModeKHR presentModes[16];
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkCtx.physicalDevice, vkCtx.surface, &presentModeCount, presentModes);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes, presentModeCount);

    VkExtent2D extent = chooseSwapExtent(&capabilities, width, height);
    vkCtx.swapchainExtent = extent;
    vkCtx.swapchainImageFormat = surfaceFormat.format;

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    vkCtx.swapchainImageCount = imageCount;

    VkSwapchainCreateInfoKHR createInfo = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    createInfo.surface = vkCtx.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(vkCtx.physicalDevice, vkCtx.surface);
    uint32_t queueFamilyIndices[] = {(uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily};
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    checkVkResult(vkCreateSwapchainKHR(vkCtx.device, &createInfo, NULL, &vkCtx.swapchain),
                  "Failed to create swapchain");

    vkGetSwapchainImagesKHR(vkCtx.device, vkCtx.swapchain, &vkCtx.swapchainImageCount, NULL);
    vkCtx.swapchainImages = malloc(sizeof(VkImage) * vkCtx.swapchainImageCount);
    vkGetSwapchainImagesKHR(vkCtx.device, vkCtx.swapchain, &vkCtx.swapchainImageCount, vkCtx.swapchainImages);

    vkCtx.swapchainImageViews = malloc(sizeof(VkImageView) * vkCtx.swapchainImageCount);
    for (uint32_t i = 0; i < vkCtx.swapchainImageCount; i++) {
        VkImageViewCreateInfo viewInfo = {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        viewInfo.image = vkCtx.swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkCtx.swapchainImageFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        checkVkResult(vkCreateImageView(vkCtx.device, &viewInfo, NULL, &vkCtx.swapchainImageViews[i]),
                      "Failed to create image view");
    }
}

// Skapa render pass med färg- och depth-bilagor
static void createRenderPass() {
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = vkCtx.swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth bilaga
    VkAttachmentDescription depthAttachment = {0};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {0};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    checkVkResult(vkCreateRenderPass(vkCtx.device, &renderPassInfo, NULL, &vkCtx.renderPass),
                  "Failed to create render pass");
}

// Skapa framebuffers
static void createFramebuffers() {
    vkCtx.framebuffers = malloc(sizeof(VkFramebuffer) * vkCtx.swapchainImageCount);
    for (uint32_t i = 0; i < vkCtx.swapchainImageCount; i++) {
        VkImageView attachments[2] = {vkCtx.swapchainImageViews[i], vkCtx.depthImageView};
        VkFramebufferCreateInfo framebufferInfo = {.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferInfo.renderPass = vkCtx.renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vkCtx.swapchainExtent.width;
        framebufferInfo.height = vkCtx.swapchainExtent.height;
        framebufferInfo.layers = 1;
        checkVkResult(vkCreateFramebuffer(vkCtx.device, &framebufferInfo, NULL, &vkCtx.framebuffers[i]),
                      "Failed to create framebuffer");
    }
}

// Skapa djupbild, minne och vy
static VkFormat findDepthFormat() {
    return VK_FORMAT_D32_SFLOAT;
}

static void createDepthResources() {
    VkExtent3D depthExtent = {vkCtx.swapchainExtent.width, vkCtx.swapchainExtent.height, 1};
    VkImageCreateInfo imageInfo = {.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = depthExtent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = findDepthFormat();
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    checkVkResult(vkCreateImage(vkCtx.device, &imageInfo, NULL, &vkCtx.depthImage), "Failed to create depth image");

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(vkCtx.device, vkCtx.depthImage, &memReqs);
    VkMemoryAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memReqs.size;
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkCtx.physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            allocInfo.memoryTypeIndex = i;
            break;
        }
    }
    checkVkResult(vkAllocateMemory(vkCtx.device, &allocInfo, NULL, &vkCtx.depthImageMemory),
                  "Failed to allocate depth image memory");
    vkBindImageMemory(vkCtx.device, vkCtx.depthImage, vkCtx.depthImageMemory, 0);

    VkImageViewCreateInfo viewInfo = {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = vkCtx.depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = findDepthFormat();
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    checkVkResult(vkCreateImageView(vkCtx.device, &viewInfo, NULL, &vkCtx.depthImageView),
                  "Failed to create depth image view");
}

// Skapa command pool och buffers
static void createCommandPool() {
    QueueFamilyIndices indices = findQueueFamilies(vkCtx.physicalDevice, vkCtx.surface);
    VkCommandPoolCreateInfo poolInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolInfo.queueFamilyIndex = indices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    checkVkResult(vkCreateCommandPool(vkCtx.device, &poolInfo, NULL, &vkCtx.commandPool),
                  "Failed to create command pool");
}

static void createCommandBuffers() {
    vkCtx.commandBuffers = malloc(sizeof(VkCommandBuffer) * vkCtx.swapchainImageCount);
    VkCommandBufferAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = vkCtx.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = vkCtx.swapchainImageCount;
    checkVkResult(vkAllocateCommandBuffers(vkCtx.device, &allocInfo, vkCtx.commandBuffers),
                  "Failed to allocate command buffers");
}

// Skapa semaphorer och fence
static void createSyncObjects() {
    VkSemaphoreCreateInfo semInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    checkVkResult(vkCreateSemaphore(vkCtx.device, &semInfo, NULL, &vkCtx.imageAvailableSemaphore),
                  "Failed to create imageAvailableSemaphore");
    checkVkResult(vkCreateSemaphore(vkCtx.device, &semInfo, NULL, &vkCtx.renderFinishedSemaphore),
                  "Failed to create renderFinishedSemaphore");
    checkVkResult(vkCreateFence(vkCtx.device, &fenceInfo, NULL, &vkCtx.inFlightFence),
                  "Failed to create inFlightFence");
}

// ---------- Huvudfunktioner ----------

static void initVulkan(int width, int height) {
    // 1) Skapa Vulkan-instans
    VkApplicationInfo appInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "VulkanApp";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    const char* extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
    VkInstanceCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 0;

    checkVkResult(vkCreateInstance(&createInfo, NULL, &vkCtx.instance), "Failed to create Vulkan instance");

    // 2) Skapa Win32-surface
    VkWin32SurfaceCreateInfoKHR surfaceInfo = {.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};

    // surfaceInfo.hwnd = window.window;
    // surfaceInfo.hinstance = GetModuleHandle(NULL);
    checkVkResult(vkCreateWin32SurfaceKHR(vkCtx.instance, &surfaceInfo, NULL, &vkCtx.surface),
                  "Failed to create Win32 surface");

    // 3) Välj fysisk enhet och skapa logisk enhet
    pickPhysicalDevice();
    createLogicalDevice(vkCtx.surface);

    // 4) Skapa swapchain + imageViews
    createSwapchain(width, height);

    // 5) Skapa depth-bilaga
    createDepthResources();

    // 6) Skapa render pass
    createRenderPass();

    // 7) Skapa framebuffers
    createFramebuffers();

    // 8) Skapa command pool och command buffers
    createCommandPool();
    createCommandBuffers();

    // 9) Skapa synkroniseringsobjekt
    createSyncObjects();
}

static void cleanupSwapchain() {
    vkDestroyImageView(vkCtx.device, vkCtx.depthImageView, NULL);
    vkDestroyImage(vkCtx.device, vkCtx.depthImage, NULL);
    vkFreeMemory(vkCtx.device, vkCtx.depthImageMemory, NULL);

    for (uint32_t i = 0; i < vkCtx.swapchainImageCount; i++) {
        vkDestroyFramebuffer(vkCtx.device, vkCtx.framebuffers[i], NULL);
    }
    free(vkCtx.framebuffers);

    vkFreeCommandBuffers(vkCtx.device, vkCtx.commandPool, vkCtx.swapchainImageCount, vkCtx.commandBuffers);
    free(vkCtx.commandBuffers);

    vkDestroyCommandPool(vkCtx.device, vkCtx.commandPool, NULL);

    vkDestroyRenderPass(vkCtx.device, vkCtx.renderPass, NULL);

    for (uint32_t i = 0; i < vkCtx.swapchainImageCount; i++) {
        vkDestroyImageView(vkCtx.device, vkCtx.swapchainImageViews[i], NULL);
    }
    free(vkCtx.swapchainImageViews);
    free(vkCtx.swapchainImages);

    vkDestroySwapchainKHR(vkCtx.device, vkCtx.swapchain, NULL);
}

static void recreateSwapchain(int width, int height) {
    vkDeviceWaitIdle(vkCtx.device);
    cleanupSwapchain();
    createSwapchain(width, height);
    createDepthResources();
    createRenderPass();
    createFramebuffers();
    createCommandBuffers();
}

int gpu_backend_vulkan_init(int width, int height) {
    initVulkan(width, height);
    return 1;
}

void gpu_backend_vulkan_prepare_frame() {
    vkWaitForFences(vkCtx.device, 1, &vkCtx.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(vkCtx.device, 1, &vkCtx.inFlightFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(vkCtx.device, vkCtx.swapchain, UINT64_MAX,
                          vkCtx.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkCommandBuffer cmd = vkCtx.commandBuffers[imageIndex];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkRenderPassBeginInfo rpInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    rpInfo.renderPass = vkCtx.renderPass;
    rpInfo.framebuffer = vkCtx.framebuffers[imageIndex];
    rpInfo.renderArea.offset = (VkOffset2D){0, 0};
    rpInfo.renderArea.extent = vkCtx.swapchainExtent;
    VkClearValue clearValues[2];
    clearValues[0].color = (VkClearColorValue){{0.2f, 0.3f, 0.3f, 1.0f}};
    clearValues[1].depthStencil = (VkClearDepthStencilValue){1.0f, 0};
    rpInfo.clearValueCount = 2;
    rpInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void gpu_backend_vulkan_render_mesh(Mesh* mesh) {
    VkCommandBuffer cmd = vkCtx.commandBuffers[0];  // alternativt aktuell index
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->vbo, offsets);
    vkCmdBindIndexBuffer(cmd, mesh->ebo, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkCtx.graphicsPipeline);
    vkCmdDrawIndexed(cmd, mesh->index_count, 1, 0, 0, 0);
}

void gpu_backend_vulkan_end_frame() {
    VkCommandBuffer cmd = vkCtx.commandBuffers[0];
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkSemaphore waitSemaphores[] = {vkCtx.imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    VkSemaphore signalSemaphores[] = {vkCtx.renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkQueueSubmit(vkCtx.graphicsQueue, 1, &submitInfo, vkCtx.inFlightFence);

    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vkCtx.swapchain;
    uint32_t imageIndex = 0;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(vkCtx.presentQueue, &presentInfo);
}

void gpu_backend_vulkan_set_framebuffer_size(int width, int height) {
    vkDeviceWaitIdle(vkCtx.device);
    recreateSwapchain(width, height);
}

VkImageView gpu_backend_vulkan_generate_brdf_lut(Shader* shader) {
    // Offscreen pass kräver eget renderpass, framebuffer etc.
    // Stub: returnerar VK_NULL_HANDLE för nu och implementera senare vid behov
    return VK_NULL_HANDLE;
}

// Glöm inte cleanup vid programavslut! (ej implementerat här)
