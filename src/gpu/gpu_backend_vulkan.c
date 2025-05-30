#include "../gpu/gpu_backend_vulkan.h"

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
} VulkanContext;

static VulkanContext vkCtx;

static void initVulkan(int width, int height, OsWindowBackend window);
static void cleanupSwapchain();
static void recreateSwapchain(int width, int height);

int gpu_backend_vulkan_init(int width, int height, OsWindowBackend window) {
    initVulkan(width, height, window);
    return 1;
}

void gpu_backend_vulkan_prepare_frame() {
    // Vänta på fence
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
    vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->vbo, (VkDeviceSize[]){0});
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
    // TODO: Implementera offscreen render pass för BRDF LUT
    return VK_NULL_HANDLE;
}

static void initVulkan(int width, int height, OsWindowBackend window) {
}
static void cleanupSwapchain() {
}
static void recreateSwapchain(int width, int height) {
}