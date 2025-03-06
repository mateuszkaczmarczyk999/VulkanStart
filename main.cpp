#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class VulanApp
{
    public:
        void run()
        {
            initWindow();
            initVulkan();
            loop();
            cleanup();
        }
    
    private:
        GLFWwindow* window;
        VkInstance instance;

        void initWindow()
        {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        }

        void initVulkan()
        {
            createInstance();
        }

        void createInstance()
        {
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Vulkan Start";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3;

            std::vector<const char*> requiredExtensions;
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            for (uint32_t i = 0; i < glfwExtensionCount; i++)
            {
                requiredExtensions.emplace_back(glfwExtensions[i]);
            }
            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
            createInfo.ppEnabledExtensionNames = requiredExtensions.data();
            createInfo.enabledLayerCount = 0;
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create instance!");
            }
        }

        void loop()
        {
            while(!glfwWindowShouldClose(window))
            {
                glfwPollEvents();
            }

        }

        void cleanup()
        {
            vkDestroyInstance(instance, nullptr);
            glfwDestroyWindow(window);
            glfwTerminate();
        }
};

int main()
{
    VulanApp app;
    try
    {
        app.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}