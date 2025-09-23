# Compiler
CC = g++
COMPILER_FLAGS = -Wall -Wextra -Wfatal-errors
LANG_STD = -std=c++17

# Paths
ifndef VULKAN_SDK
    $(error "VULKAN_SDK is not set. Run 'source /opt/VulkanSDK/1.4.304.1/setup-env.sh' and try again.")
endif
GLFW_PATH = /opt/homebrew/opt/glfw
GLM_PATH = /opt/homebrew/opt/glm

# Linking Flags
INCLUDE_PATHS = -I$(VULKAN_SDK)/include -I$(GLFW_PATH)/include -I$(GLM_PATH)/include
LINKER_PATHS = -L$(VULKAN_SDK)/lib -L$(GLFW_PATH)/lib
LINKER_FLAGS = -lvulkan -lglfw

# Source files
SOURCE = main.cpp VulkanRenderer.cpp
TARGET = vulkan_app

###########################################################
# RULES
###########################################################

build:
	${CC} ${COMPILER_FLAGS} ${LANG_STD} ${INCLUDE_PATHS} ${SOURCE} ${LINKER_PATHS} ${LINKER_FLAGS} -o ${TARGET};

run:
	./${TARGET};

clean:
	rm -f ${TARGET};