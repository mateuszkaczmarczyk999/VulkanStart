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
    pickPhysicalDevice();
    createLogicalDevice();
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

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
           deviceFatures.shaderInt64 &&
           queueIndices.isReady();
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
    deviceCreateInfo.enabledExtensionCount = 0;

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
