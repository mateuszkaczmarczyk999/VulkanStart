#include "VulkanRenderer.hpp"

VulkanRenderer::VulkanRenderer()
{
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::initialize()
{
    createWindow();
    createInstance();
}

void VulkanRenderer::run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void VulkanRenderer::flush()
{
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char *> *extensionsToCheck)
{
    // Get number of supported extensions
    uint32_t supportedExtensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionsCount, nullptr);

    // Get supported extensions
    std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionsCount, supportedExtensions.data());

    // Check if extensions are supported
    for (const auto &extensionName : *extensionsToCheck)
    {
        bool extensionFound = false;
        for (const auto &supported : supportedExtensions)
        {
            if (std::strcmp(extensionName, supported.extensionName) == 0)
            {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound)
        {
            return false;
        }
    }

    return true;
}

void VulkanRenderer::createWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
}

void VulkanRenderer::createInstance()
{
    // Application info structure
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // Collecting required extensions
    std::vector<const char *> requiredExtensions;
    uint32_t glfwExtensionsCount = 0;
    const char *const *glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
    for (uint32_t i = 0; i < glfwExtensionsCount; i++)
    {
        requiredExtensions.push_back(glfwExtensions[i]);
    }
    requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    if (!checkInstanceExtensionSupport(&requiredExtensions))
    {
        throw std::runtime_error("Not all required extensions are supported!");
    }

    // Instance info structure
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    // Create Vulkan instance
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }
}
