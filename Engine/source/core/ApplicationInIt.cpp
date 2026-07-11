#include "core/Application.h"

/**
 * @brief Static callback function triggered by Vulkan Validation Layers.
 * * This function acts as the "receiver" for GPU/Driver error messages. It captures
 * the validation data and outputs it to the standard error stream.
 *
 * @param messageSeverity The severity of the message (Verbose, Info, Warning, Error).
 * @param messageType The category of the message (General, Validation, Performance).
 * @param pCallbackData Structure containing the detailed error message and object handles.
 * @param pUserData Optional user-defined pointer passed through from the messenger setup.
 *
 * @return VkBool32 Returns VK_FALSE to indicate the call should not be aborted.
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	[[maybe_unused]] void* pUserData) {

	// Print the message to the standard error stream
	std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE; // Always return false
}

/**
 * @brief Application constructor.
 * Orchestrates the high-level initialization of the windowing system and
 * the Vulkan graphics API.
 */
Application::Application() {
	initWindow();
	initVulkan();
}

/**
 * @brief Application destructor.
 * Ensures an orderly shutdown by calling the cleanup routine.
 */
Application::~Application() {
	cleanup();
}

/**
 * @brief Initializes the permanent Debug Messenger for the Blitz Engine.
 * This messenger monitors API calls throughout the application lifecycle.
 *
 * @param createInfo The structure to be initialized and populated.
 */
void Application::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	// Since this is an extension, we use the Volk-loaded function
	if (vkCreateDebugUtilsMessengerEXT(m_vulkinstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger!");
	}
}

/**
 * @brief Populates a messenger create info struct with standard settings. Centralizes the configuration for message severity and type filtering.
 *
 * @return A vector of C-string names for the required instance extensions.
 */
void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {}; // Zero-initialize the struct
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	// Filter which severities we want to hear about
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	// Filter which types of messages we care about
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	// Link the static callback function we wrote
	createInfo.pfnUserCallback = debugCallback;

	// Optional: Pointer to pass custom data to the callback (we don't need it yet)
	createInfo.pUserData = nullptr;
}


/**
 * @brief Initializes the GLFW window.
 * Sets window hints to disable the default OpenGL context and creates
 * the platform-specific window handle.
 */
void Application::initWindow() {
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW");

	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //Telling GLFW not OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);

	if (!m_window) {
		throw std::runtime_error("Failed to create window");
	}
}

/**
 * @brief Initializes the Vulkan library and core engine handles.
 * Uses Volk to load function pointers, creates the instance, and
 * initializes the debug messenger.
 */
void Application::initVulkan() {
	//Initialize the Volk loader first to get standard Vulkan functions
	if (volkInitialize() != VK_SUCCESS) {
		throw::std::runtime_error("Failed to initialize Vulkan");
	}

	Application::createInstance();

	volkLoadInstance(m_vulkinstance);

	setupDebugMessenger();

	createSurface();

	pickPhysicalGPU();
	createLogicalDevice();

	createSwapChain();
	createImageViews();
	createGraphicsPipeline();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects(); // Implement this function block to bake the fences!
}

/**
 * @brief Creates the Vulkan Instance.
 * Acts as the primary handshake between Blitz and the graphics driver.
 * Defines application identity, required extensions, and validation layers.
 */
void Application::createInstance() {

	// =========================================================================
	// 1. APPLICATION INFO (The "Identity Card")
	// =========================================================================
	// This struct tells the graphics driver (Nvidia/AMD/Intel) exactly what 
	// software is running. Drivers often read the ApplicationName and EngineName 
	// to apply specific, behind-the-scenes performance optimizations for your app.
	//Pipeline: appInfo -> OS -> GPU
	VkApplicationInfo appInfo{};

	//In Vulkan, almost every struct requires me to explicitly state it's own type
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

	appInfo.pApplicationName = "TradingPlatform3D"; //Name of the end product
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	appInfo.pEngineName = "blitz";

	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	appInfo.apiVersion = VK_API_VERSION_1_3;

	// =========================================================================
	// 2. INSTANCE CREATE INFO (The "Blueprint")
	// =========================================================================
	// This struct is mandatory. It tells the Vulkan runtime which global 
	// extensions and layers you want to activate before the Instance is built.
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	createInfo.pApplicationInfo = &appInfo;

	// =========================================================================
	// 3. EXTENSIONS (The "Translators")
	// =========================================================================
	// Vulkan is *platform-agnostic. It doesn't know what Windows or a "Screen" is.
	// We must ask GLFW which Vulkan extensions are required to allow the GPU 
	// to talk to the Windows OS windowing system.

	//*Platform-agnostic meaning:Platform agnostic refers to software, hardware, or services designed to operate across multiple operating systems, devices, 
	// or environments without requiring significant reconfiguration.

	auto glfwExtensions = getRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();


	// =========================================================================
	// 4. VALIDATION LAYERS (The "Safety Net")
	// =========================================================================
	// Check if we are in Debug mode and if the system actually supports our layers.
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

	if (enableValidationLayers) {
		if (!checkValidationLayerSupport()) {
			throw std::runtime_error("Validation layers requested, but not supported on local machine.");
		}
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// =========================================================================
	// 5. EXECUTION (The "Handshake")
	// =========================================================================
	// Send the blueprint (&createInfo) to the driver. 
	// The 'nullptr' is for custom memory allocators (we don't need them yet).
	// If successful, it populates your m_instance variable with a valid handle.
	if (vkCreateInstance(&createInfo, nullptr, &m_vulkinstance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan Instance");
	}
}

/**
 * @brief Gathers all extensions required by GLFW and the engine itself.
 * Vulkan requires explicit activation of platform-specific surface extensions and debug utilities.
 *
 * Requires VK_EXT_debug_utils to be enabled during Instance creation.
 */
std::vector<const char*> Application::getRequiredExtensions() {

	// 1. Get basic extensions GLFW needs to communicate with windows.
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// 2. Convert the C-style array from GLFW into C++ vector for easier handling.
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

/**
 * @brief Binds the native window instance context to a Vulkan Surface.
 * Calls GLFW to execute platform-specific handshakes, establishing an abstract
 * rendering surface link target for frame output operations.
 */
void Application::createSurface() {
	//GLFW will handle the surface creation
	if (glfwCreateWindowSurface(m_vulkinstance, m_window, nullptr, &m_surface)) {
		throw std::runtime_error("Unable to create surface!");
	}
}


/**
 * @brief Checks for the presence of requested validation layers.
 * Enumerates all layers installed on the system and verifies that
 * the Khronos validation suite is available.
 * @return True if all required layers are supported, false otherwise.
 */
bool Application::checkValidationLayerSupport() {
	// 1. Find out how many layers are available on this local
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// 2. Next, allocate a vector large enough to hold all available layer properties
	std::vector<VkLayerProperties> availableLayers(layerCount);

	// 3. Fill the vector with the actual layer data
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {

			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) return false;
	}

	return true;
}

/**
 * @brief Starts the application's primary execution cycle.
 */
void Application::run() {
	Application::mainLoop();
}

/**
 * @brief The engine's heart. Polls OS events until the window is closed.
 */
void Application::mainLoop() {
	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();	//To keep the window responsive
		drawFrame();			//Continous calls for CPU-GPU sync pump!
	}

	//Before the window closes, wait for GPU to completely finish activer operations
	//before start triggering our deconstructors in cleanup()
	vkDeviceWaitIdle(m_logicalDevice);
}


/**
 * @brief Orderly destruction of all engine-allocated resources.
 * Destroys handles in the reverse order of their creation to maintain
 * Vulkan's strict dependency chain.
 */
void Application::cleanup() {
	// 1. Destroy Image Views FIRST while the device is still alive!
	for (auto imageView : m_swapChainImageViews) {
		if (imageView != VK_NULL_HANDLE) {
			vkDestroyImageView(m_logicalDevice, imageView, nullptr);
		}
	}
	m_swapChainImageViews.clear(); // Empty the container safely

	// 2. Clear out your validation layers/debug utilities
	if (enableValidationLayers) {
		vkDestroyDebugUtilsMessengerEXT(m_vulkinstance, m_debugMessenger, nullptr);
	}

	// 3. Clean up for graphics pipeline & command pool
	if (m_graphicsPipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
	}

	if (m_pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
	}

	if (m_commandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
	}

	// === ADD YOUR NEW SYNCHRONIZATION CLEANUP FOR PIPELINES LOOP HERE ===
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (m_imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
			vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphores[i], nullptr);
			m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
		}
		if (m_inFlightFences[i] != VK_NULL_HANDLE) {
			vkDestroyFence(m_logicalDevice, m_inFlightFences[i], nullptr);
			m_inFlightFences[i] = VK_NULL_HANDLE;
		}
	}

	// Loop 2: SAFE FIX - Loop directly through the semaphore array size itself!
	for (size_t i = 0; i < m_renderFinishedSemaphores.size(); i++) {
		if (m_renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
			vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphores[i], nullptr);
			m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
		}
	}
	m_renderFinishedSemaphores.clear();
	// 4. NOW it is safe to destroy the swapchain and device itself
	if (m_swapChain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);
	}

	if (m_logicalDevice != VK_NULL_HANDLE) {
		vkDestroyDevice(m_logicalDevice, nullptr);
	}
	// 5. Finally, tear down instance level dependencies and windows
	if (m_surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(m_vulkinstance, m_surface, nullptr);
	}

	if (m_vulkinstance != VK_NULL_HANDLE) {
		vkDestroyInstance(m_vulkinstance, nullptr);
	}

	if (m_window) {
		glfwDestroyWindow(m_window);
	}

	glfwTerminate();
}