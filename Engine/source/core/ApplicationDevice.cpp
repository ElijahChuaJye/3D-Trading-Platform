/**
* @file Application.cpp
* @brief Main engine orchestration and Vulkan pipeline implementation.
** @author Elijah Chua Jye Kang (elijahchua64@gmail.com)
* @date Started : September 2025
* *@description
* This file serves as the heart of the Blitz engine, responsible for the
* orchestration of the Vulkan graphics API. It manages the lifecycle of the
* application, including window management, hardware interrogation,
* logical device initialization, and the setup of the rendering pipeline.
* */

// 1. FIRST: Include your application header. 
// WHY: This pulls in 'volk.h' first, ensuring all Vulkan structures and 
// functions are fully loaded in memory before VMA tries to use them.
#include "core/Application.h"

// 2. SECOND: Set up the VMA implementation macro.
#define VMA_IMPLEMENTATION

// 3. THIRD: Tell MSVC to temporarily turn off all warnings.
// WHY: VMA is a highly generic, cross-platform library. Wrapping the include 
// in a warning push/pop block tells the Microsoft compiler to drop its warning 
// level to '0' (complete silence) ONLY for this header, preventing our strict 
// '/WX' (warnings-as-errors) flag from triggering on AMD's code.
#if defined(_MSC_VER)
#pragma warning(push, 0) 
#endif

// 4. FOURTH: Include the allocator code now that warnings are muted.
#include <vk_mem_alloc.h>

// 5. FIFTH: Instantly restore our strict '/W4 /WX' warning levels.
// WHY: This ensures that any code we write below this line is still strictly 
// scrutinized by the compiler, maintaining our high code-quality standards.
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

/**
* @brief Picking the physical GPU that will be used for Vulkan
*
* @return Nothing but it picks the phsical GPU that vulkan will use for the grahics
*/
void Application::pickPhysicalGPU() {
	uint32_t deviceCount{}; //Stores the total count of phtsical device discovered

	// 1. Ask Vulkan how many gpus are there in this local machine first
	vkEnumeratePhysicalDevices(m_vulkinstance, &deviceCount, nullptr);

	//If it has zero GPU, vulkan won't be able to run
	if (deviceCount == 0) {
		throw std::runtime_error("Zero GPU detected in local device!");
	}

	// 2. Allocate an array(vector) big enough to hold all the GPU discovered
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_vulkinstance, &deviceCount, devices.data());

	// 3. Create a multimap to automatically sort our GPU based on perfromance score
	// std::multimap sorts key n ascending order automatically (based on performance)
	std::multimap<int, VkPhysicalDevice> candidates;

	// 4. Interrogate every single GPU found on the machine
	for (auto const& device : devices) {
		int score = rateGPUSuitability(device);

		candidates.insert(std::make_pair(score, device));
	}

	//If the any of the candidate score is more than 0
	if (candidates.rbegin()->first > 0) {
		m_physicalDevice = candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("Failed to find a matching GPU for the engine.");
	}

}

/**
 * @brief Initializes the driver-linked logical device context interface.
 * Queries unique execution queues needed for presentation and graphics pipelines, enables
 * low-level hardware structures, binds core extension array dependencies (like the swapchain),
 * triggers device initialization, and fetches queue processing handles via Volk.
 */
void Application::createLogicalDevice() {

	// 1. Find the queue families indices first supported by our chosen GPU
	QueueFamilies indices = findHardwareFamily(m_physicalDevice);

	// 2. Set the unique queue families to prevent duplication from happening
	std::vector<VkDeviceQueueCreateInfo> createQueueInfo;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	// 3. Assigning performance priority (0.0 - 1.0) to our queues
	float queuePiority{ 1.0f };
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePiority;

		createQueueInfo.push_back(queueCreateInfo);

	}

	// 4. Speicfy the low-level for hardware features we want to activate
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.geometryShader = VK_TRUE;

	//This is for dynamic rendering
	VkPhysicalDeviceVulkan13Features features13{};
	features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	features13.dynamicRendering = VK_TRUE; // Explicitly request dynamic rendering
	features13.pNext = nullptr;

	// 5. Fill the primary logical device configuration device
	VkDeviceCreateInfo createInfo{};

	createInfo.pNext = &features13;
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	//Fix for the exception thrown
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(createQueueInfo.size());
	createInfo.pQueueCreateInfos = createQueueInfo.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// 6. Pass queue number setup and to enable extensions
	std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	//7. If the validations is enabled, validate with older drivers
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	//8. Actual creation of the logical device 
	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device!");
	}

	// Load device-specific function pointers through Volk
	volkLoadDevice(m_logicalDevice);

	//9. Retrival: Snatch the handles back	to command queues
	vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, indices.presentFamily.value(), 0, &m_presentQueue); //  Corrected!

}


/********************************************
*
* HELPER FUNCTIONS
*
********************************************/

/**
* Helper function to find the hardware within the local machine to find the correct hardware
*
**/
QueueFamilies Application::findHardwareFamily(VkPhysicalDevice& device) {
	QueueFamilies indicies;

	//1. Get the number of queue family supported by this GPU
	uint32_t queueFamilyCount{};
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	//2. Allocate space and retrieve properties of all queue families
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int idx{};

	for (const auto& queueMembers : queueFamilies) {

		//Check if this specific family gear supports GRAPHICS command
		if (queueMembers.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indicies.graphicsFamily = idx;
		}

		//Check if this specific queue family supports PRESENTATION(Showing image to our window)
		//Requires m_surface to be intiailized first
		VkBool32 presentSupport{ false };
		vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, m_surface, &presentSupport);

		if (presentSupport) {
			indicies.presentFamily = idx;
		}

		if (indicies.isComplete()) {
			break;
		}

		++idx;
	}

	return indicies;
}

/**
* Helper function for the picking of GPU device based on score
*
*
**/
int Application::rateGPUSuitability(VkPhysicalDevice device) {
	// 1. Asking for high-level GPU's name, type, properties and limit
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// 2. Asking GPU for low-level GPU features (Geomtert Shaders
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// 3. Find the available hardware departments (Queues)
	QueueFamilies indices = findHardwareFamily(device);

	//CRITICAL RULE:
	//If it lacks graphics department, a display presentation department,
	// or geometry shaders, it is immidately scored as 0 as they those are very important.
	if (!indices.graphicsFamily || !indices.presentFamily || !deviceFeatures.geometryShader) {
		return 0;

	}

	int score{};

	//PERFORMACE PREFERENCE: Discrete GPUs gets a massive point boost.
	//Dedicated GPU card will score higher than integrated Intel GPU card
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 2212;
	}

	score += deviceProperties.limits.maxImageDimension2D;

	return score;

}