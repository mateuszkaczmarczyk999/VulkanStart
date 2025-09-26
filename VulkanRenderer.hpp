#ifndef VULKANRENDERER_HPP
#define VULKANRENDERER_HPP

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <optional>
#include <set>

const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 600;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentationFamily;

    bool isReady()
    {
        return graphicsFamily.has_value() && presentationFamily.has_value();
    }
};

class VulkanRenderer
{
public:
    VulkanRenderer();
    ~VulkanRenderer();
    void initialize();
    void run();
    void flush();

private:
    void createWindow();
    bool checkValidationLayerSupport(std::vector<const char *> *validationLayersToCheck);
    bool checkInstanceExtensionSupport(std::vector<const char *> *extensionsToCheck);
    void createInstance();
    void setupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void createSurface();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> *extensionsToCheck);
    bool isDeviceSuitable(VkPhysicalDevice device);
    void pickPhysicalDevice();
    void createLogicalDevice();

    GLFWwindow *window;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;
};

#endif // VULKANRENDERER_HPP
