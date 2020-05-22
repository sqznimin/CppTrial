#include "RenderVulkan/VulkanRootSignature.h"
#include "RenderVulkan/VulkanDevice.h"
#include "RenderVulkan/VulkanDescriptorSetLayout.h"

namespace RenderCore
{

VulkanRootSignature::VulkanRootSignature(const RootSignatureDesc &desc) : RootSignature(desc)
{
    Array<VkDescriptorSetLayout> setLayouts;
    for(auto &e : desc.layouts)
    {
        auto vkLayout = static_cast<VulkanDescriptorSetLayout *>(e.get());
        setLayouts.Add(vkLayout->GetHandle());
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayouts.Count();
    pipelineLayoutInfo.pSetLayouts = setLayouts.GetData();
    if(vkCreatePipelineLayout(gVulkanDevice->GetLogicalDeviceHandle(), &pipelineLayoutInfo, gVulkanAlloc, &pipelineLayout) != VK_SUCCESS)
        CT_EXCEPTION(RenderCore, "Create pipeline layout failed.");
}

VulkanRootSignature::~VulkanRootSignature()
{
    if(pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(gVulkanDevice->GetLogicalDeviceHandle(), pipelineLayout, gVulkanAlloc);
        pipelineLayout = VK_NULL_HANDLE;
    }
}

}