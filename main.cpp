#include "VulkanRenderer.hpp"
#include <stdexcept>
#include <iostream>

int main()
{
    VulkanRenderer vulkanRenderer;
    try
    {
        vulkanRenderer.initialize();
        vulkanRenderer.run();
        vulkanRenderer.flush();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}