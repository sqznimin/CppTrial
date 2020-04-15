#pragma once

#include "RenderVulkan/VulkanMemory.h"

namespace RenderCore
{
class VulkanDevice : public IVulkanResource
{
public:
    VulkanDevice(VkPhysicalDevice device);
    ~VulkanDevice();

    virtual void Destroy() override;

    auto GetGraphicsQueueFamilyIndex() const
    {
        return graphicsQueueData.familyIndex;
    }

    VkPhysicalDevice GetPhysicalDeviceHandle() const
    {
        return physicalDevice;
    }

    VkDevice GetLogicalDeviceHandle() const
    {
        return logicalDevice;
    }

    static SPtr<VulkanDevice> Create(VkPhysicalDevice device);

private:
    struct QueueData
    {
        uint32 familyIndex;
        Array<SPtr<VulkanQueue>> queues;
    };

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    QueueData graphicsQueueData;
};
}