#include "VulkanRenderer.hpp"

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger)
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

static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
    VkDebugUtilsMessageTypeFlagsEXT msgType,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData)
{
    std::cerr << "[ VALIDATION LAYER: ] " << callbackData->pMessage << std::endl;
    return VK_FALSE;
}

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
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();
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
    vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool VulkanRenderer::checkValidationLayerSupport(std::vector<const char *> *validationLayersToCheck)
{
    // Get number of supported validation layers
    uint32_t supportedValidationLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&supportedValidationLayerCount, nullptr);

    // Get supported validation layers
    std::vector<VkLayerProperties> supportedValidationLayers(supportedValidationLayerCount);
    vkEnumerateInstanceLayerProperties(&supportedValidationLayerCount, supportedValidationLayers.data());

    // Check if layers are supported
    for (const auto &layerName : *validationLayersToCheck)
    {
        bool layerFound = false;
        for (const auto &supported : supportedValidationLayers)
        {
            if (std::strcmp(layerName, supported.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            return false;
        }
    }

    return true;
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
    // Preparing validation layers
    std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    if (enableValidationLayers && !checkValidationLayerSupport(&validationLayers))
    {
        throw std::runtime_error("Not all validation layers are supported!");
    }

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
    if (enableValidationLayers)
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    if (!checkInstanceExtensionSupport(&requiredExtensions))
    {
        throw std::runtime_error("Not all required extensions are supported!");
    }

    // Instance info structure
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
    if (enableValidationLayers)
    {
        instanceInfo.enabledLayerCount = (uint32_t)validationLayers.size();
        instanceInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(messengerCreateInfo);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&messengerCreateInfo;
    }
    else
    {
        instanceInfo.enabledLayerCount = 0;
    }

    // Create Vulkan instance
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create instance!");
    }
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &messengerCreateInfo)
{
    messengerCreateInfo = {};
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerCreateInfo.pfnUserCallback = debugCallback;
    messengerCreateInfo.pUserData = nullptr;
}

void VulkanRenderer::setupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
    populateDebugMessengerCreateInfo(messengerCreateInfo);

    VkResult result = CreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo, nullptr, &debugMessenger);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create debug messenger!");
    }
}

void VulkanRenderer::createSurface()
{
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create surface window!");
    }
}

void VulkanRenderer::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    for (const auto &device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find suitable GPU!");
    }
}

bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFatures);

    QueueFamilyIndices queueIndices = findQueueFamilies(device);

    std::vector<const char *> deviceExtensions = {
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    bool extensionsSupported = checkDeviceExtensionSupport(device, &deviceExtensions);

    bool swapchainAdequate = false;
    if (extensionsSupported)
    {
        SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
        swapchainAdequate = !swapchainSupport.surfaceFormats.empty() && !swapchainSupport.surfacePresentModes.empty();
    }

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
           deviceFatures.shaderInt64 &&
           queueIndices.isReady() &&
           extensionsSupported &&
           swapchainAdequate;
}

SwapchainSupportDetails VulkanRenderer::querySwapchainSupport(VkPhysicalDevice device)
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.surfaceCapabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.surfaceFormats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.surfacePresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.surfacePresentModes.data());
    }

    return details;
}

bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> *extensionsToCheck)
{
    uint32_t extensionsCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

    std::vector<VkExtensionProperties> supportedExtensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, supportedExtensions.data());

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

QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

    QueueFamilyIndices queueIndices;
    int queueIdx = 0;
    for (const auto &properties : queueFamilyProperties)
    {
        VkBool32 presentationSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, queueIdx, surface, &presentationSupport);
        if (presentationSupport)
        {
            queueIndices.presentationFamily = queueIdx;
        }
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueIndices.graphicsFamily = queueIdx;
        }
        if (queueIndices.isReady())
        {
            break;
        }
        queueIdx++;
    }

    return queueIndices;
}

void VulkanRenderer::createLogicalDevice()
{
    std::vector<const char *> deviceExtensions = {
        "VK_KHR_portability_subset",
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        queueIndices.graphicsFamily.value(),
        queueIndices.presentationFamily.value(),
    };
    float priority = 1.0f;

    for (const auto queueFamilyIndex : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &priority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.shaderInt64 = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // if (enableValidationLayers)
    // {
    //     deviceCreateInfo.enabledLayerCount = (uint32_t)(validationLayers.size());
    //     deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    // }
    // else
    // {
    //     deviceCreateInfo.enabledLayerCount = 0;
    // }

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(logicalDevice, queueIndices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, queueIndices.presentationFamily.value(), 0, &presentationQueue);
}

VkSurfaceFormatKHR VulkanRenderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &available : availableFormats)
    {
        if (available.format == VK_FORMAT_B8G8R8A8_SRGB && available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::choosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &available : availablePresentModes)
    {
        if (available == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
        };

        actualExtent.width = std::clamp(
            actualExtent.width,
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width);

        actualExtent.height = std::clamp(
            actualExtent.height,
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanRenderer::createSwapchain()
{
    SwapchainSupportDetails swapchainSupport = querySwapchainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapchainSupport.surfaceFormats);
    VkPresentModeKHR presentMode = choosePresentMode(swapchainSupport.surfacePresentModes);
    VkExtent2D swapExtent = chooseSwapExtent(swapchainSupport.surfaceCapabilities);

    uint32_t imageCount = swapchainSupport.surfaceCapabilities.minImageCount + 1;
    if (swapchainSupport.surfaceCapabilities.maxImageCount > 0 && imageCount > swapchainSupport.surfaceCapabilities.maxImageCount)
    {
        imageCount = swapchainSupport.surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {queueIndices.graphicsFamily.value(), queueIndices.presentationFamily.value()};

    if (queueIndices.graphicsFamily != queueIndices.presentationFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapchainSupport.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapchainImages.data());
    
    swapchainFormat = surfaceFormat.format;
    swapchainExtent = swapExtent;
}
