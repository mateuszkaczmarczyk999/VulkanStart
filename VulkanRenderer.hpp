#ifndef VULKANRENDERER_HPP
#define VULKANRENDERER_HPP

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>

const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 600;

class VulkanRenderer
{
public:
    VulkanRenderer();
    ~VulkanRenderer();
    void initialize();
    void run();
    void flush();

private:
    bool checkInstanceExtensionSupport(std::vector<const char *> *extensionsToCheck);
    void createWindow();
    void createInstance();

    GLFWwindow *window;
    VkInstance instance;
};

#endif // VULKANRENDERER_HPP
