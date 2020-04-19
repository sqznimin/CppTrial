#include "Core/String.h"
#include "Core/Exception.h"
#include "IO/FileHandle.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "RenderVulkan/VulkanContext.h"
#include "RenderVulkan/VulkanSwapChain.h"
#include "RenderVulkan/VulkanRenderPipeline.h"
#include "RenderVulkan/VulkanSync.h"
#include "RenderVulkan/VulkanCommandBuffer.h"
using namespace RenderCore;

const int32 WIDTH = 800;
const int32 HEIGHT = 600;

GLFWwindow *window;
SPtr<VulkanRenderPipelineState> pipelineState;
Array<SPtr<VulkanCommandBuffer>> commandBuffers;

SPtr<Shader> CreateShader()
{
    ShaderCreateParams params;
    IO::FileHandle vertSrcFile(CT_TEXT("External/vulkan/shader.vert"));
    IO::FileHandle fragSrcFile(CT_TEXT("External/vulkan/shader.frag"));
    params.vertexSource = vertSrcFile.ReadString();
    params.fragmentSource = fragSrcFile.ReadString();
    return Shader::Create(params);
}

void InitVulkan()
{
    auto &context = VulkanContext::Get();
    context.Init();

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(context.GetInstanceHandle(), window, gVulkanAlloc, &surface) != VK_SUCCESS)
        CT_EXCEPTION(LearnVK, "Create surface failed.");
 
    VkBool32 supportsPresent;
    auto physicalDevice = context.GetDevice()->GetPhysicalDeviceHandle();
    auto familyIndex = context.GetDevice()->GetGraphicsQueueFamilyIndex();
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &supportsPresent);
    if (!supportsPresent)
        CT_EXCEPTION(LearnVK, "Graphics queue cannot support presentation.");

    context.SetSurfaceKHR(surface);

    //Swap chain
    VulkanSwapChainCreateParams swapChainParams;
    swapChainParams.width = WIDTH;
    swapChainParams.height = HEIGHT;
    context.RecreateSwapChain(swapChainParams);


    RenderPipelineStateCreateParams pipelineStateParams;
    BlendStateCreateParams blendParams;
    pipelineStateParams.blendState = BlendState::Create(blendParams);
    DepthStencilStateCreateParams depthStencilParams;
    pipelineStateParams.depthStencilState = DepthStencilState::Create(depthStencilParams);
    RasterizationStateCreateParams rasterizationParams;
    pipelineStateParams.rasterizationState = RasterizationState::Create(rasterizationParams);
    pipelineStateParams.shader = CreateShader();
    pipelineState = std::static_pointer_cast<VulkanRenderPipelineState>(RenderPipelineState::Create(pipelineStateParams));

}

void CleanupVulkan()
{
    auto &context = VulkanContext::Get();
    context.Destroy();
}

int main(int argc, char **argv)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "LearnVK", nullptr, nullptr);

    InitVulkan();

    auto &context = VulkanContext::Get();
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        auto &frameData = context.GetCurrentFrameData();

        frameData.fence->Wait();
        context.GetSwapChain()->AcquireBackBuffer();
        frameData.fence->Reset();

        context.SetFrameBuffer(context.GetSwapChain()->GetCurentBackBufferData().frameBuffer);
        context.SetRenderPipelineState(pipelineState);

        auto backBufferIndex = context.GetSwapChain()->GetCurrentBackBufferIndex();
        if (commandBuffers.Count() <= backBufferIndex)
        {
            auto commandBuffer = context.GetRenderCommandPool()->GetIdleBuffer();
            commandBuffer->Begin();
            commandBuffer->BeginRenderPass();
            commandBuffer->BindRenderPipeline();
            commandBuffer->SetViewport(0, 0, WIDTH, HEIGHT);
            commandBuffer->SetScissor(0, 0, WIDTH, HEIGHT);
            commandBuffer->Draw(0, 3, 1);
            commandBuffer->EndRenderPass();
            commandBuffer->End();

            commandBuffers.Add(commandBuffer);
        }

        const auto &commandBuffer = commandBuffers[backBufferIndex];
        context.Submit(commandBuffer);
        context.Present();
    }

    CleanupVulkan();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}