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
#include <glm/glm.hpp>
#include <optional>
#include <vector>
#include <array>
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

struct Vertex {
	glm::vec2 pos;	//2D Position
	glm::vec3 color;//RGB color

	//Vulkan needs to know how step through C++ array in GPU memory.
	//This informs the GPU's input assembler how larget each "packet" is
	//so it does not go out-of-bounds or misalign memory addresses.
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;

		//Tells the PUT to jump forward by the correct size of our vertex structure
		//to find the next elemen, preventing memory overlapping (stride)
		bindingDescription.stride = sizeof(Vertex);

		//Step per-vertex rather than per-instance because we are rendering
		//unique geometry points, not duplicating memory overlapping.
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	//VkBuffer is just a raw blob of binary bytes, since Vulkan has no idea
	//where 'pos' ends and 'color' begins. This function tells the shdaer complier
	//how to extract and unpack those variables from ra streams.
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		//Attribute 0: Position
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;// Mayches layout(location = 0) in vertex.vert shader

		//VK_FORMAT_R32G32_SFLOAT represents a 2D vector (2 * 32 bit floats)
		//Even though it uses "RBA" color-based naming, Vulakns uses these formats
		//as generic data size templates to save API complexity
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;

		//Offsetof dynamically calculates how many bytes 'pos' is from the start 
		//start of struct, protecting us if we change the struct member order
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		// Attribute 1: Color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1; // WHY: Matches layout(location = 1) in vertex.vert shader

		// WHY: Three 32-bit floats (R, G, B) mapping directly to a vec3 in our shader.
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};



#endif // !VULKTYPES_H
