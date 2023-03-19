#pragma once
#include "Helgelse/CreateWindow.hpp"
#include "FSNG/Path.hpp"
#include "FSNG/Data.hpp"
#include "FSNG/Forge/Forge.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <magic_enum.hpp>

#include <tuple>
#include <vector>
#include <string>

using namespace FSNG;

auto glfw_init() -> bool {
    if(glfwInit()==GLFW_FALSE)
		return false;

    // check for vulkan support
	if(glfwVulkanSupported()==GLFW_FALSE) {
		// not supported
		glfwTerminate();
		return false;
	}
	return true;
}

auto glfw_create_window(auto const &applicationName, auto width, auto height) -> std::optional<GLFWwindow*> {
    // create window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);		// This tells GLFW to not create an OpenGL context with the window
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	if(auto window = glfwCreateWindow(width, height, applicationName, nullptr, nullptr))
		return window;
	return std::nullopt;
}

auto create_application_info(auto const &applicationName) -> VkApplicationInfo {
	VkApplicationInfo application_info;
	application_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion			= VK_MAKE_VERSION( 1, 0, 2 );
	application_info.applicationVersion	= VK_MAKE_VERSION( 0, 0, 1 );
	application_info.engineVersion		= VK_MAKE_VERSION( 0, 0, 1 );
	application_info.pApplicationName	= applicationName;
	application_info.pEngineName		= applicationName;
    return application_info;
}

auto vulkan_create_instance(auto const &instance_layers, auto const &instance_extensions, auto const &applicationName) -> std::optional<VkInstance> {
	auto application_info = create_application_info(applicationName);

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType					 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo		 = &application_info;
	instance_create_info.enabledLayerCount		 = 0;
	//instance_create_info.enabledLayerCount	 = instance_layers.size();
	//instance_create_info.ppEnabledLayerNames	 = instance_layers.data();
	instance_create_info.enabledExtensionCount	 = instance_extensions.size();
	instance_create_info.ppEnabledExtensionNames = instance_extensions.data();
	instance_create_info.flags                  |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	VkInstance instance	= VK_NULL_HANDLE;
	auto const result = vkCreateInstance(&instance_create_info, nullptr, &instance);
	if(result != VK_SUCCESS) {
		std::cout << "Error from Vulkan during vkCreateInstance: " << magic_enum::enum_name(result) << std::endl;
		return std::nullopt;
	}
    return instance;
}

auto vulkan_create_device(auto const &GPUs, auto const &device_extensions, auto const &graphics_queue_family) -> std::optional<VkDevice> {
	const float priorities[] {1.0f};
	VkDeviceQueueCreateInfo queue_create_info{};
	queue_create_info.sType			   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueCount	   = 1;
	queue_create_info.queueFamilyIndex = graphics_queue_family;
	queue_create_info.pQueuePriorities = priorities;

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType				   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount	   = 1;
	device_create_info.pQueueCreateInfos	   = &queue_create_info;
//	device_create_info.enabledLayerCount	   = device_layers.size();				// depricated
//	device_create_info.ppEnabledLayerNames	   = device_layers.data();				// depricated
	device_create_info.enabledLayerCount	   = device_extensions.size();
	device_create_info.ppEnabledExtensionNames = device_extensions.data();

	VkDevice device	= VK_NULL_HANDLE;
	auto const result = vkCreateDevice(GPUs[0], &device_create_info, nullptr, &device);
	if(result != VK_SUCCESS) {
		std::cout << "Error from Vulkan during vkCreateDevice: " << magic_enum::enum_name(result) << std::endl;
		return std::nullopt;
	}
    return device;
}

auto vulkan_setup_extensions() -> std::tuple<std::vector<const char*>, std::vector<const char*>, std::vector<const char*>> {
    // regular instance and device layers and extensions
	std::vector<const char*> instance_layers;
	//	std::vector<const char*> device_layers;					// depricated
	std::vector<const char*> instance_extensions;
	std::vector<const char*> device_extensions;

    // if using debugging, push back debug layers and extensions
	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	//	device_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );			// depricated

    // push back extensions and layers you need
	// We'll need the swapchain for sure if we want to display anything
	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Get required instance extensions to create the window.
	// These instance extensions change from OS to OS.
	// For example on Windows we'd get back "VK_KHR_surface" and "VK_KHR_win32_surface"
	// and on Linux XCB window library we'd get back "VK_KHR_surface" and "VK_KHR_xcb_surface"
	uint32_t instance_extension_count		 = 0;
	const char ** instance_extensions_buffer = glfwGetRequiredInstanceExtensions(&instance_extension_count);
	for(uint32_t i=0; i < instance_extension_count; ++i) {
		// Push back required instance extensions as well
		instance_extensions.push_back( instance_extensions_buffer[ i ] );
	}
	instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	return std::make_tuple(instance_layers, instance_extensions, device_extensions);
}

auto vulkan_setup_gpus(auto const &instance) -> std::optional<std::vector<VkPhysicalDevice>>{
    // Get GPUs
	uint32_t GPU_count;
	vkEnumeratePhysicalDevices(instance, &GPU_count, nullptr);
	std::vector<VkPhysicalDevice> GPUs( GPU_count );
	vkEnumeratePhysicalDevices(instance, &GPU_count, GPUs.data());

	return GPUs;
}

auto vulkan_setup_graphics_queue_family(auto const &GPUs) -> std::optional<uint32_t> {
    // select graphics queue family
	uint32_t queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties( GPUs[ 0 ], &queue_family_count, nullptr );
	std::vector<VkQueueFamilyProperties> family_properties( queue_family_count );
	vkGetPhysicalDeviceQueueFamilyProperties( GPUs[ 0 ], &queue_family_count, family_properties.data() );

    uint32_t graphicsQueueFamily = UINT32_MAX;
	for(uint32_t i=0; i < queue_family_count; ++i)
		if(family_properties[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			graphicsQueueFamily = i;
	if( graphicsQueueFamily == UINT32_MAX ) {
		// queue family not found
		glfwTerminate();
		return std::nullopt;
	}
	// graphicsQueueFamily now contains queue family ID which supports graphics
	return graphicsQueueFamily;
}

auto glfw_create_surface(auto const &instance, auto const &window, auto width, auto height) -> std::optional<VkSurfaceKHR> {
    // make sure we indeed get the surface size we want.
	glfwGetFramebufferSize(window, &width, &height);

    // Create window surface, looks a lot like a Vulkan function ( and not GLFW function )
	// This is a one function solution for all operating systems. No need to hassle with the OS specifics.
	// For windows this would be vkCreateWin32SurfaceKHR() or on linux XCB window library this would be vkCreateXcbSurfaceKHR()
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkResult ret = glfwCreateWindowSurface( instance, window, nullptr, &surface );
	if(VK_SUCCESS != ret) {
		// couldn't create surface, exit
		glfwTerminate();
		return std::nullopt;
	}
	return surface;
}

auto glfw_poll_loop(auto const &window) {
    /*
	All regular Vulkan API stuff goes here, no more GLFW commands needed for the window.
	We still need to initialize the swapchain, it's images and all the rest
	just like we would have done with OS native windows.
	*/
	while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

auto vulkan_terminate(auto const &instance, auto const &surface, auto const &window, auto const &device) {
	// Destroy window surface, Note that this is a native Vulkan API function
	// ( surface was created with GLFW function )
	if(instance != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(instance, surface, nullptr);

	// destroy window using GLFW function
	glfwDestroyWindow(window);

	// destroy Vulkan device and instance normally
	if(device != VK_NULL_HANDLE)
		vkDestroyDevice(device, nullptr);
	if(instance != VK_NULL_HANDLE)
		vkDestroyInstance(instance, nullptr);
}

auto vulkan_init(auto const &applicationName, auto const &window, auto width, auto height) -> bool {
	auto [instance_layers, instance_extensions, device_extensions] = vulkan_setup_extensions();

    VkInstance instance = VK_NULL_HANDLE;
	if(auto instanceOpt = vulkan_create_instance(instance_layers, instance_extensions, applicationName))
		instance = instanceOpt.value();
	else
		return false;

	std::vector<VkPhysicalDevice> GPUs;
	if(auto GPUsOpt = vulkan_setup_gpus(instance))
		GPUs = GPUsOpt.value();
	else
		return false;

	uint32_t graphicsQueueFamily;
	if(auto const graphicsQueueFamilyOpt = vulkan_setup_graphics_queue_family(GPUs))
		graphicsQueueFamily = graphicsQueueFamilyOpt.value();
	else
		return false;

    VkDevice device = VK_NULL_HANDLE;
	if(auto const deviceOpt = vulkan_create_device(GPUs, device_extensions, graphicsQueueFamily))
		device = deviceOpt.value();
	else
		return false;

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if(auto const surfaceOpt = glfw_create_surface(instance, window, width, height))
		surface = surfaceOpt.value();
	else
		return false;

	glfw_poll_loop(window);

    return true;
}

auto glfw_terminate() {
	glfwTerminate();
}

namespace Helgelse {
struct GLFWVulkanSpace {
    ~GLFWVulkanSpace() {
        Forge::instance()->clearBlock(*this->root);
    }

    auto operator==(GLFWVulkanSpace const &rhs) const -> bool { }

    virtual auto grabBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        return false;
    }

    virtual auto grab(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        return false;
    }

    virtual auto read(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
        return false;
    }

    virtual auto readBlock(Path const &range, std::type_info const *info, void *data, bool isTriviallyCopyable) -> bool {
		return false;
    }

    virtual auto insert(Path const &range, Data const &data, Path const &coroResultPath="") -> bool {
        if(range.spaceName()=="windows") {
			auto const applicationName = "GLFW with Vulkan";
			int width  = 800;
			int height = 600;
			if(!this->isGLFWInitialized)
				if(!glfw_init())
					return false;
			if(auto const windowOpt = glfw_create_window(applicationName, width, height); auto window = windowOpt.value()) {
				if(!this->isVulkanInitialized)
					if(!vulkan_init(applicationName, window, width, height)) {
						vulkan_terminate(instance, surface, window, device);
						return false;
					}
			}
			return true;
        }
        return false;
    }

    virtual auto toJSON() const -> nlohmann::json {
        nlohmann::json json;
        return json;
    }
private:
    PathSpaceTE *root=nullptr;
	bool isGLFWInitialized = false;
	bool isVulkanInitialized = false;
	std::map<std::string, GLFWwindow*> windows;

	VkInstance instance = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
};
}