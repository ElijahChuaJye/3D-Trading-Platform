/**
 * @file VulkTypes.h
 * @brief Fundamental Vulkan data structure definitions and hardware abstractions.
 * * @author Elijah Chua Jye Kang (elijahchua64@gmail.com)
 * @date Started: September 2025
 * * @description
 * This file contains the foundational POD (Plain Old Data) structures used by
 * the Blitz engine to interface with Vulkan. It abstracts hardware queue
 * discovery results and swapchain support capabilities, providing type-safe
 * containers that enable the engine to perform disciplined hardware
 * interrogation before initializing the rendering pipeline.
 */

#ifndef VULKTYPES_H
#define VULKTYPES_H
#include <volk/volk.h>
#include <optional>
#include <vector>
#include <cstdint>

 /**
 * @brief Trakcs the indicies of the GPU's requirement "departments"
 * Vulkan not only draws but also present a window for the drawing.
 * There will be a need to find the correct department to handle the specific jobs.
 */
struct QueueFamilies {
	//Optional is being used because index 0 is valid, so we can't use -1 as "not found"
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	//Helper function to ensure everything needed to for the blitz
	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;		//Max/Min count, window size
	std::vector<VkSurfaceFormatKHR> formats;	//Color/Depth formats
	std::vector<VkPresentModeKHR> presentModes;	//Refresh Logic (V-Sync, etc.)
};


#endif // !VULKTYPES_H
