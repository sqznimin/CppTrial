#include "RenderVulkan/VulkanContext.h"
#include "RenderVulkan/VulkanQueue.h"
#include "RenderVulkan/VulkanSync.h"
#include "RenderVulkan/VulkanSwapChain.h"
#include "RenderVulkan/VulkanRenderPipeline.h"
#include "RenderVulkan/VulkanFrameBuffer.h"
#include "RenderVulkan/VulkanCommandBuffer.h"
#include "RenderVulkan/VulkanShader.h"

namespace RenderCore
{
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    CT_LOG(Debug, CT_TEXT("Validation layer: {0}"), String(pCallbackData->pMessage));
    return VK_FALSE;
}

void VulkanContext::Init()
{
#if CT_DEBUG
    enableValidationLayers = true;
#endif

    VulkanShaderCompiler::Get().Init();

    CreateInstance();
    CreateDebugger();
    CreateDevice();
    CreateFrameDatas();
    CreateCommandPools();
}

void VulkanContext::Destroy()
{
    shaderRegistry.Cleanup();

    DestroyCommandPools();
    DestroyFrameDatas();

    for (auto &e : renderPipelines)
        e->Destroy();
    renderPipelines.Clear();

    if (swapChain)
        swapChain->Destroy();
    if (surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(instance, surface, gVulkanAlloc);

    device->Destroy();

    if (enableValidationLayers && debugMessenger != VK_NULL_HANDLE)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, gVulkanAlloc);
    vkDestroyInstance(instance, gVulkanAlloc);

    VulkanShaderCompiler::Get().Destroy();
}

void VulkanContext::RecreateSwapChain(const VulkanSwapChainCreateParams &params)
{
    if (!swapChain)
        swapChain = VulkanSwapChain::Create();
    swapChain->Recreate(params);
}

void VulkanContext::Submit(const SPtr<VulkanCommandBuffer> &buffer)
{
    auto &frameData = frames[currentFrameIndex];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {frameData.swapChainImageSemaphore->GetHandle()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    auto commandBuffer = buffer->GetHandle();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {frameData.renderFinishedSemaphore->GetHandle()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(buffer->GetQueue()->GetHandle(), 1, &submitInfo, frameData.fence->GetHandle()) != VK_SUCCESS)
        CT_EXCEPTION(RenderCore, "Submit failed.");
}

void VulkanContext::Present()
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    VkSemaphore waitSemaphores[] = {frames[currentFrameIndex].renderFinishedSemaphore->GetHandle()};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = waitSemaphores;

    uint32 imageIndex = swapChain->GetCurrentBackBufferIndex();
    VkSwapchainKHR swapChains[] = {swapChain->GetHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(device->GetGraphicsQueue()->GetHandle(), &presentInfo);

    currentFrameIndex = (currentFrameIndex + 1) % VULKAN_FRAME_NUM;
}

SPtr<VulkanRenderPipeline> VulkanContext::GetRenderPipeline()
{
    if (!frameBuffer)
    {
        CT_EXCEPTION(RenderCore, "Current frame buffer is null.");
        return nullptr;
    }
    if (!renderPipelineState)
    {
        CT_EXCEPTION(RenderCore, "Current pipeline state is null.");
        return nullptr;
    }

    auto renderPass = frameBuffer->GetRenderPass();
    for (const auto &e : renderPipelines)
    {
        if (e->IsMatch(renderPass))
            return e;
    }

    VulkanRenderPipelineCreateParams params;
    params.renderPass = renderPass;
    params.state = renderPipelineState;
    SPtr<VulkanRenderPipeline> result = VulkanRenderPipeline::Create(params);
    renderPipelines.Add(result);
    return result;
}

void VulkanContext::CreateInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "CppTrial";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "CppTrialFramework";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    Array<const char8 *> extensions;
    extensions.Add(VK_KHR_SURFACE_EXTENSION_NAME);
#if CT_PLATFORM_WIN32
    extensions.Add("VK_KHR_win32_surface");
#endif
    if (enableValidationLayers)
        extensions.Add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    Array<const char8 *> validationLayers;
    if (enableValidationLayers)
        validationLayers.Add("VK_LAYER_LUNARG_standard_validation");

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.Count();
    createInfo.ppEnabledExtensionNames = extensions.GetData();
    createInfo.enabledLayerCount = validationLayers.Count();
    createInfo.ppEnabledLayerNames = validationLayers.GetData();

    if (vkCreateInstance(&createInfo, gVulkanAlloc, &instance) != VK_SUCCESS)
        CT_EXCEPTION(RenderCore, "Create instance failed.");
}

void VulkanContext::CreateDebugger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, gVulkanAlloc, &debugMessenger) != VK_SUCCESS)
        CT_EXCEPTION(RenderCore, "Create debugger failed.");
}

void VulkanContext::CreateDevice()
{
    Array<VkPhysicalDevice> devices;
    uint32 deviceCount = 0;

    if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) == VK_SUCCESS && deviceCount > 0)
    {
        devices.AppendUninitialized(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.GetData());

        int32 selected = 0;
        if (deviceCount > 1)
        {
            for (int32 i = 0; i < devices.Count(); ++i)
            {
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(devices[i], &props);
                if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    selected = i;
                    break;
                }
            }
        }

        device = VulkanDevice::Create(devices[selected]);
    }
    else
        CT_EXCEPTION(RenderCore, "Enumerate physical devices failed.");
}

void VulkanContext::CreateFrameDatas()
{
    for (int32 i = 0; i < VULKAN_FRAME_NUM; ++i)
    {
        frames[i].renderFinishedSemaphore = VulkanSemaphore::Create();
        frames[i].swapChainImageSemaphore = VulkanSemaphore::Create();
        frames[i].fence = VulkanFence::Create();
    }
}

void VulkanContext::CreateCommandPools()
{
    VulkanCommandPoolCreateParams params;
    params.familyIndex = device->GetGraphicsQueueFamilyIndex();
    params.queue = device->GetGraphicsQueue();
    renderCommandPool = VulkanCommandPool::Create(params);
}

void VulkanContext::DestroyFrameDatas()
{
    for (int32 i = 0; i < VULKAN_FRAME_NUM; ++i)
    {
        frames[i].renderFinishedSemaphore->Destroy();
        frames[i].swapChainImageSemaphore->Destroy();
        frames[i].fence->Destroy();
    }
}

void VulkanContext::DestroyCommandPools()
{
    renderCommandPool->Destroy();
}

}