#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <vulkan/vulkan.h>
#include "nisaba/gpu/gpu_device.hpp"

namespace nisaba::gpu {

class VulkanDevice : public GpuDevice {
public:
    static std::shared_ptr<VulkanDevice> create();
    static std::shared_ptr<VulkanDevice> create_headless(uint32_t width = 800, uint32_t height = 600);

    ~VulkanDevice() override;

    [[nodiscard]] GpuBackendType backend_type() const noexcept override {
        return GpuBackendType::Vulkan;
    }

    void make_current() override;
    std::unique_ptr<Context> create_context(int flags = 0) override;

    void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    void set_scissor(const std::optional<ScreenIntRect>& scissor) override;
    void set_blend_mode(BlendMode blend_mode) override;
    void clear(nisaba::Color color) override;
    void read_pixels(int32_t x, int32_t y, uint32_t width, uint32_t height, void* out_rgba) override;

    // Vulkan specific accessors
    [[nodiscard]] VkInstance instance() const noexcept { return instance_; }
    [[nodiscard]] VkPhysicalDevice physical_device() const noexcept { return physical_device_; }
    [[nodiscard]] VkDevice device() const noexcept { return device_; }
    [[nodiscard]] VkQueue graphics_queue() const noexcept { return graphics_queue_; }
    [[nodiscard]] uint32_t graphics_queue_family() const noexcept { return graphics_queue_family_; }
    [[nodiscard]] VkCommandPool command_pool() const noexcept { return command_pool_; }

    // Memory helpers
    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    bool create_buffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& out_buffer,
        VkDeviceMemory& out_memory
    ) const;

    bool create_image(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkSampleCountFlagBits samples,
        VkImage& out_image,
        VkDeviceMemory& out_memory
    ) const;

    VkCommandBuffer begin_single_time_commands() const;
    void end_single_time_commands(VkCommandBuffer cmd) const;

public:
    VulkanDevice();

private:

    VkInstance instance_{VK_NULL_HANDLE};
    VkPhysicalDevice physical_device_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE};
    VkQueue graphics_queue_{VK_NULL_HANDLE};
    uint32_t graphics_queue_family_{0};
    VkCommandPool command_pool_{VK_NULL_HANDLE};

    bool init_vulkan();
};

} // namespace nisaba::gpu
