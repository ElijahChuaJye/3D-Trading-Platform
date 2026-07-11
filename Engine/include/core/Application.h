/**
 * @file Application.h
 * @brief Class definition for the Blitz engine core interface.
 * * @author Elijah Chua Jye Kang (elijahchua64@gmail.com)
 * @date Started: September 2025
 * * @description
 * Defines the Application class, which encapsulates the Vulkan instance,
 * physical/logical devices, window management, and rendering lifecycle.
 * This header acts as the central interface for engine initialization
 * and resource cleanup.
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <vulkan/volk.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <string>
#include <vector>
#include <stdexcept>
#include <map>
#include <set>
#include <iostream>

#include "VulkTypes.h"

class Application {
public:
	/// <summary>
	/// Ctor
	/// </summary>
	Application();

	/// <summary>
	/// dtor
	/// </summary>
	~Application();

	/// <summary>
	/// Delete the copy ctor of the Application
	/// </summary>
	/// <param name=""> Another type Application instance </param>
	Application(const Application&) = delete;

	/// <summary>
	/// Delete the assign copy constructor
	/// </summary>
	/// <param name=""></param>
	/// <returns> Nothing </returns>
	Application& operator=(const Application&) = delete;

	/// <summary>
	/// To run the main application loop
	/// </summary>
	void run();


private:
	// --- Life Cycle --
	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanup();

	// --- Vulkan Helpers --
	void createInstance();
	bool checkValidationLayerSupport();
	void pickPhysicalGPU();
	void createSurface();

	// -- Vulkan instruction and command pool --
	/// <summary>
	/// Allocates a persistent execution pool from the GPU's graphics queue family.
	/// This pool acts as the backing memory manager for all our command buffers.
	/// </summary>
	void createCommandPool();

	/// <summary>
	/// Allocates individual command buffers from our command pool (one per swapchain image).
	/// These act as the individual script pages where we record our GPU macros.
	/// </summary>
	void createCommandBuffers();

	/// <summary>
	/// Compiles our SPIR-V shaders and bakes the fixed-function pipeline states.
	/// Uses Vulkan 1.3 Dynamic Rendering to bypass traditional render pass objects.
	/// </summary>
	void createGraphicsPipeline();

	/// <summary>
	/// Allocates the hardware fences and semaphores needed to safely coordinate 
	/// asynchronous frame processing between the CPU and GPU.
	/// </summary>
	void createSyncObjects();

	// --- Core Runtime Drawing Stages ---

	/// <summary>
	/// Coordinates the CPU-GPU frame synchronization pump. 
	/// Acquires a swapchain image, triggers recording, submits work to the queue, and presents the frame.
	/// </summary>
	void drawFrame();

	/// <summary>
	/// Records the explicit hardware instructions into a target command buffer.
	/// Handles image memory layout transitions and wraps draw calls inside a dynamic rendering block.
	/// </summary>
	/// <param name="commandBuffer">The execution script handle currently being written to.</param>
	/// <param name="imageIndex">The index of the active swapchain target texture canvas.</param>
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	/**
	 * @brief Gathers all extensions required by GLFW and the engine itself.
	 *
	 * @return A vector of C-style strings containing the extension names.
	 */
	std::vector<const char*> getRequiredExtensions();

	// Debug Messenger Helpers
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);
	void setupDebugMessenger();

	// --- Member Variables ---
	// Windowing System
	GLFWwindow* m_window{ nullptr };              // Platform-specific window handle (managed by GLFW)

	// Core Vulkan Instance & Debugging
	VkInstance m_vulkinstance{ VK_NULL_HANDLE };  // Primary interface to the Vulkan library
	VkDebugUtilsMessengerEXT m_debugMessenger{ VK_NULL_HANDLE }; // Callback messenger for validation layer logs

	// Physical Hardware
	VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE }; // Handle to the selected GPU silicon

	// Logical Device & Queues
	VkDevice m_logicalDevice{ VK_NULL_HANDLE };   // The logical interface/connection to the GPU
	VkQueue m_graphicsQueue{ VK_NULL_HANDLE };    // Command queue for graphics-specific operations (rendering)
	VkQueue m_presentQueue{ VK_NULL_HANDLE };     // Command queue for display presentation (swapchain)

	//VkInstance is the connection between my engine and Vulkan Library.
	//It is a  member variable because it will be used to create all the things we need.
	VkInstance m_creator{};

	//Vulkan needs us to specificly tell them what OS screen to be drawn on unlike GLFW which
	//automatically handles the OS screen surface
	VkSurfaceKHR m_surface{ nullptr };

	//Vulkan commanding member variables
	VkCommandPool m_commandPool{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> m_commandBuffers{}; // One per swapchain image

	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline m_graphicsPipeline{ VK_NULL_HANDLE };

	std::vector<VkSemaphore> m_imageAvailableSemaphores;	// GPU-side: Signals when swapchain texture is ready
	std::vector<VkSemaphore> m_renderFinishedSemaphores;	// GPU-side: Signals when graphics processing completes
	std::vector<VkFence>     m_inFlightFences;				// CPU-side: Gates CPU loop to match GPU execution speed

	const int MAX_FRAMES_IN_FLIGHT = 2; // Keep 2 frames moving at once
	uint32_t m_currentFrame = 0;        // Track the active loop index
	//For the screen
	const int m_width = 1280;
	const int m_height = 720;
	const std::string m_title = "blitz - TradingPlatform3D";

	/**
	 * @brief List of validation layers to be enabled
	 * VK_LAYERS_KHRONOS_validation is the standard suite that checks for common
	 * API misuse, memory leaks and threading issues.
	 */
	const std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };

	/**
	* @brief Helper function to find the actual hardware that will be needed by Vulkan.
	*
	*/
	QueueFamilies findHardwareFamily(VkPhysicalDevice& device);

	/**
	* @brief Rates the score of the GPU present in the local device
	*
	**/
	int rateGPUSuitability(VkPhysicalDevice device);

	/**
	* @brief TO create a logicial device helper function. This function is used to connect vulkan and hardware,
	* telling the GPU what to do
	*
	**/
	void createLogicalDevice();

	/**
	* @brief Function to query about the swap frame buffer
	*
	*/
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& devce);


	/**
	 * @brief Toggle for validation layers based on build configuration.
	 * NDEBUG is a standard macro defined by compliers in 'Release' mode.
	 */
#ifdef NDEBUG
	const bool enableValidationLayers = false;	//Disable for performance in Release mode
#else
	const bool enableValidationLayers = true;	//Enable for safety in Debug mode
#endif

	//For the SwapChain. since Vulkan does not have it's own, I will need to manually create teh swap buffer with screen we found
	//through the devices function we made
	VkSwapchainKHR m_swapChain{ VK_NULL_HANDLE };
	std::vector<VkImage> m_swapChainImages{};
	VkFormat m_swapChainImageFormat{};
	VkExtent2D m_swapChainExtent{};
	std::vector<VkImageView> m_swapChainImageViews{};


	//Helper Swap chain functions
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaliableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	//Chain functions
	void createSwapChain();
	void createImageViews();

};


#endif
