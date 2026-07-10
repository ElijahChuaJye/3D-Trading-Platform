#include "core/Application.h"

//Helper functions for Swap chain

/**
* @brief Checks if the monitors supports SRGB color space
**/
VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	//Look through the list of formats with GPU supports
	for (const auto& availableFormat : availableFormats) {

		//We want SRGB color space with 8-bit per channel color format (RGB)
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	//If there isn't just use the first format
	return availableFormats[0];
}

/**
* @brief V-SYnc behavior. Used to render as fast but simply replaces the old frame if the monitor
* isn't ready yet, ensuring ultra low-latency without screen tearing
**/
VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		//Mail box mode allwoing low-latency rendering without tearing
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	//Fifo is the standard V-sync. Locks framerate to monitor's refresh rate.
	//Vulkan spec guarantees that FIFO is always available on every device.
	return VK_PRESENT_MODE_FIFO_KHR;
}

/**
* @brief Calculate the exact pixel dimensions of the images we are creating
**/
VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites) {
	//If the driver specfies the exact resolution, use it directly
	if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilites.currentExtent;
	}
	else {
		//Otherwise ask GLFW for actual pixel dimensions of the window
		int width{}, height{};

		glfwGetFramebufferSize(m_window, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width),
		static_cast<uint32_t>(height) };

		actualExtent.width = std::clamp<uint32_t>(actualExtent.width,
			capabilites.minImageExtent.width,
			capabilites.maxImageExtent.width);

		actualExtent.height = std::clamp<uint32_t>(actualExtent.height,
			capabilites.minImageExtent.height,
			capabilites.maxImageExtent.height);

		return actualExtent;
	}

}

/**
* @brief Function to query about the swap frame buffer
*/
SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice& device) {
	SwapChainSupportDetails dets{};

	//1. Get basic surface capabilites
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &dets.capabilities);

	//2. Surface formats
	uint32_t formatCount{};
	//Check first to ensure that there are formats first
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
	if (formatCount != 0) {
		//Once confirmed that there are formats from the surface, proceeed to store them
		dets.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, dets.formats.data());
	}

	//3. Present modes
	uint32_t presentMode{};
	//Check if there are present modes present first
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentMode, nullptr);
	if (presentMode != 0) {
		dets.presentModes.reserve(presentMode);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentMode, dets.presentModes.data());
	}

	return dets;


}

/**
 * @brief Allocates and configures the virtual swapchain system.
 * Evaluates hardware features to choose optimal image layouts, determines the target size
 * of the canvas bounds, specifies structural queue ownership parameters, allocates the
 * underlying images, and caches configuration properties for the graphics pipeline.
 */
void Application::createSwapChain() {
	//1. Query the physical GPU to see the capabilites that the current monitor/surface support
	SwapChainSupportDetails swapChainSupport{ querySwapChainSupport(m_physicalDevice) }; //m_physicalevce is referring to the selected GPU

	//2. Setting selection
	//Use helper functions to pick the best color format, presentation mode and resolution
	VkSurfaceFormatKHR surfaceFormat{ chooseSwapSurfaceFormat(swapChainSupport.formats) };
	VkPresentModeKHR presentMode{ chooseSwapPresentMode(swapChainSupport.presentModes) };
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	//3. Request one more image than he minimum to prevent waiting on the driver
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	//4. Swapchain blueprint
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;

	//Core canvas setting
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//5. Queue syncronaization logic
	//Check if the silicon lane that draws the Graphics is the same lane that shows the image (Present)
	QueueFamilies indices{ findHardwareFamily(m_physicalDevice) };
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		//Different lanes: Image must be explictly shared between queues to avoid memory access violations
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		//Same lane for optimaization for drawing and presenting happening on the same queue
		//Exclusive mode provides the highest performance
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	//6. Presentation & OS Window Settings
	//Thelling the swap chain if we want to physically rotate the image(But we will just use the screen's default)
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	//Ignore alpha blending with other OS desktop windows behind ours (Creates a solid, opaque window)
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	//Set presentation mode
	createInfo.presentMode = presentMode;

	//ALlow GPU to skip rendering pixels that are obscured by other windows overlapping
	createInfo.clipped = VK_TRUE;

	//If window resizes, a new chain swap must be made. Points to the old one to transfer ownership smoothly
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	//7. EXECUTION
	//Hand the completed blueprint of the information to the driver to allocate the Swap Chain Object
	if (vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain");
	}

	//8. Retrieve the raw image handles
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, nullptr);

	//Resize the vector to hold the size
	m_swapChainImages.resize(imageCount);

	//Pull the raw VkImage handles out of the GPU and store them in the vecotr
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, m_swapChainImages.data());

	//8. Cache the pipeline variables
	//Save the setting because the future graphics pipeline needs to know the exact format
	// and size of the canvas it is expected to draw onto
	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

/**
* @brief Create image view for all swap chain images.
* * Iterates through the raw VkImage handles acquired from the Swap Chain
* and wraps them VkImageView objects. These views act as "lenses" that
* instruct the GPU's graphics pipeline to treat the raw memory as 2D color
* textures, allowing the engine to safely render pixels to the screen buffer.
*
*/
void Application::createImageViews() {
	//1. Resize the container to match the exact number of swap chain
	m_swapChainImageViews.resize(m_swapChainImages.size());

	//2. Iterate and create view for raw image
	for (size_t idx{}; idx < m_swapChainImages.size(); ++idx) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapChainImages[idx];

		//Treat images as standard 2D texture
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		//Use the format cached during the swap chain creation
		createInfo.format = m_swapChainImageFormat;

		//3. Component mapping using swizzling
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		//4/ Subresource range
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		// No mipmapping or multiple layers needed for a standard 2D swap chain
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// 5. Execution
		if (vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &m_swapChainImageViews[idx]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image views!");
		}
	}
}