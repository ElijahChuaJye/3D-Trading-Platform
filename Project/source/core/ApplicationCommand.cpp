//Standard libraires
#include <fstream>

//Other header files
#include "core/Application.h"

/**
 * @brief Helper function to load compiled SPIR-V bytecode directly from the local disk.
 * Opens a file stream in binary mode, verifies that the file layout aligns cleanly
 * with 32-bit word structures, and streams raw binary data directly into a C++ memory buffer.
 * * @param fileName The physical file path relative to the executable working directory.
 * @return A vector of 32-bit unsigned integers representing the compiled SPIR-V code.
 */
std::vector<uint32_t> readShaderFile(const std::string& fileName) {
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open shader file: " + fileName);
	}

	//Ensuring is SPIR-V coded 
	size_t fileSize = (size_t)file.tellg();
	if (fileSize % 4 != 0) {
		throw std::runtime_error("Shader file size is invalid: " + fileName);
	}

	// Allocate the 32-bit vector immediately
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	// Rewind and read directly into the uint32_t memory space
	file.seekg(0);

	// OPTIMIZATION: Read directly from disk straight into our 32-bit vector.
	// Reinterpret_cast tells ifstream to treat our uint32_t destination as raw bytes just for the stream.
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();

	return buffer;
}

/**
 * @brief Constructs the immutable master graphics pipeline asset on the GPU.
 * Compiles SPIR-V shader modules, configures fixed-function pipeline states
 * (rasterization, blending, multisampling), defines the global layout binding space,
 * and binds modern Vulkan 1.3 Dynamic Rendering attachments to dictate runtime color formats.
 */
void Application::createGraphicsPipeline() {
	// Load your shaders using your highly optimized function
	auto vertShaderCode = readShaderFile("SPV/vert.spv");
	auto fragShaderCode = readShaderFile("SPV/frag.spv");

	// Vertex Shader Module Setup
	VkShaderModuleCreateInfo vertCreateInfo{};
	vertCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertCreateInfo.codeSize = vertShaderCode.size() * sizeof(uint32_t);
	vertCreateInfo.pCode = vertShaderCode.data();

	VkShaderModule vertShaderModule;
	if (vkCreateShaderModule(m_logicalDevice, &vertCreateInfo, nullptr, &vertShaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex shader module!");
	}

	// Fragment Shader Module Setup
	VkShaderModuleCreateInfo fragCreateInfo{};
	fragCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragCreateInfo.codeSize = fragShaderCode.size() * sizeof(uint32_t);
	fragCreateInfo.pCode = fragShaderCode.data();

	VkShaderModule fragShaderModule;
	if (vkCreateShaderModule(m_logicalDevice, &fragCreateInfo, nullptr, &fragShaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create fragment shader module!");
	}

	// Assign Vertex Shader to the pipeline stages
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	// Assign Fragment Shader to the pipeline stages
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// 2. DYNAMIC STATES
	std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// 3. VERTEX INPUT STATE (Empty for milestone 1)
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// 4. INPUT ASSEMBLY STATE
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// 5. VIEWPORT STATE
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// 6. RASTERIZATION STATE
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	// 7. MULTISAMPLING STATE
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// 8. COLOR BLENDING STATE
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	// 9. PIPELINE LAYOUT
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	if (vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// 10. VULKAN 1.3 DYNAMIC RENDERING RUNTIME ATTACHMENT DEFINITION
	VkPipelineRenderingCreateInfo renderingCreateInfo{};
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingCreateInfo.colorAttachmentCount = 1;
	renderingCreateInfo.pColorAttachmentFormats = &m_swapChainImageFormat;

	// 11. COMPLETE MASTER GRAPHICS PIPELINE ASSEMBLY
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = &renderingCreateInfo;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = VK_NULL_HANDLE;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// Destroy temporary shader modules once the pipeline is safely compiled onto the GPU
	vkDestroyShaderModule(m_logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(m_logicalDevice, vertShaderModule, nullptr);
}

/**
 * @brief Allocates the global command execution pool container.
 * Binds the memory allocation context directly to the active hardware graphics queue family,
 * configuring flags to allow individual command buffers to be explicitly wiped and reset
 * at the beginning of each frame lifecycle.
 */
void Application::createCommandPool() {
	QueueFamilies queueFamilyIndices = findHardwareFamily(m_physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	//Reset the command to allow us to reuse the same buffer every frame 
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool");
	}
}

/**
 * @brief Allocates standalone Primary Command Buffers out of the global pool.
 * Resizes the storage container to match the total image capacity of the active swapchain,
 * establishing a dedicated pipeline script lane for each image canvas to enable parallel
 * frame execution setups.
 */
void Application::createCommandBuffers() {
	//1. Resizing of the container to match the exact number of swapchains
	m_commandBuffers.resize(m_swapChainImages.size());

	//2. Configure the allocation blueprint
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandPool;

	//LEVEL_PRIMARY meaning this buffer can be submitte directly to a hardware
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

	//3. Request the handles from the logical device
	if (vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate comman buffers!");
	}
}

/**
 * @brief Records hardware rendering instructions into a target command buffer.
 * injects pipeline layout execution barriers to safely cycle textures through presentation
 * layouts, establishes clear color definitions, opens a modern Vulkan 1.3 dynamic rendering region,
 * defines a runtime viewport/scissor layout, and fires the final drawing trigger call.
 * * @param commandBuffer The active command buffer lane being recorded into.
 * @param imageIndex The active target canvas index currently provided by the swapchain.
 */
void Application::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {

	// =========================================================================
	// STEP 1: OPENING THE COMMAND BUFFER SCRIPT
	// =========================================================================
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; // Identifies the structure type for the Vulkan driver

	// This line initiates recording mode on the target command buffer allocation. 
	// Think of it like turning on a macro recording software or opening a new script file.
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	// =========================================================================
	// STEP 2: HARDWARE BARRIER — TRANSITION SWAPCHAIN IMAGE TO AN ACTIVE CANVAS
	// =========================================================================
	// Images acquired from the swapchain start in an "UNDEFINED" layout (garbage/unknown state).
	// The GPU hardware memory controller must reorganize its internal caching/tiling strategy 
	// to match a layout optimized for writing colors.
	VkImageMemoryBarrier barrierToDraw{};
	barrierToDraw.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // Explicit structure identification
	barrierToDraw.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;          // Memory layout coming into the barrier
	barrierToDraw.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Memory layout leaving the barrier (ready for color output)

	// These tell Vulkan if we are passing ownership of this image across different hardware execution queues.
	// Since our graphics queue handles both drawing and presenting, we leave these ignored.
	barrierToDraw.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrierToDraw.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrierToDraw.image = m_swapChainImages[imageIndex]; // Points to the specific hardware canvas texture we want to manipulate

	// SubresourceRange defines exactly which parts of our target texture are affected by this memory modification
	barrierToDraw.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // We are targeting the color channel (not depth or stencil)
	barrierToDraw.subresourceRange.baseMipLevel = 0;                       // Start at the primary mipmap layer
	barrierToDraw.subresourceRange.levelCount = 1;                         // Only apply to this 1 single mip level
	barrierToDraw.subresourceRange.baseArrayLayer = 0;                     // Start at the first texture layer array index
	barrierToDraw.subresourceRange.layerCount = 1;                         // Only apply to this single layer

	barrierToDraw.srcAccessMask = 0; // Defines what memory operations must finish *before* the barrier hits (0 means nothing to wait on)
	barrierToDraw.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Block subsequent operations from writing colors until this barrier finishes

	// This is the actual hardware command that triggers the memory reorganization.
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,             // Wait Stage: Execute this barrier at the absolute start of the GPU's pipeline work
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // Block Stage: Freeze the color writing hardware until this barrier is 100% complete
		0,
		0, nullptr,                                    // Used for abstract memory barriers (unneeded here)
		0, nullptr,                                    // Used for buffer memory barriers (unneeded here)
		1, &barrierToDraw                              // Pass our 1 explicit image barrier layout configuration
	);

	// =========================================================================
	// STEP 3: DEFINING RUNTIME COLOR PROPERTIES (DYNAMIC RENDERING ATTACHMENT)
	// =========================================================================
	// This defines the behavior of the specific target canvas *during* this rendering block execution.
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = m_swapChainImageViews[imageIndex];          // Binds the specific lens/view we are looking through
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Confirms the layout matches our hardware shift above
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                   // Tells the tile cache to wipe itself clean before drawing
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;                 // Tells the cache to save all modifications permanently to VRAM when finished
	colorAttachment.clearValue.color = { {0.01f, 0.01f, 0.015f, 1.0f} };      // Set the clean slate canvas color (Matte Dark Charcoal Blue)

	// =========================================================================
	// STEP 4: DEFINING RENDERING WINDOW AREA BOUNDARIES
	// =========================================================================
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = { {0, 0}, m_swapChainExtent }; // Defines the bounding box (from 0,0 to our window resolution width/height)
	renderingInfo.layerCount = 1;                           // Standard 2D output (not rendering a 3D texture array or cube map)
	renderingInfo.colorAttachmentCount = 1;                 // We are only outputting to 1 single color canvas attachment slot
	renderingInfo.pColorAttachments = &colorAttachment;     // Binds our color attachment behavior settings struct above

	// =========================================================================
	// STEP 5: THE DYNAMIC DRAWING SECTION (THE GPU WORK BLOCK)
	// =========================================================================
	// This officially marks the beginning of our rendering block. Traditional Vulkan required vkCmdBeginRenderPass here.
	// Vulkan 1.3 completely shortcuts it by opening an immediate dynamic drawing region.
	vkCmdBeginRendering(commandBuffer, &renderingInfo);

	// Binds our pre-compiled, immutable Graphics Pipeline asset. This activates our shaders and fixed-function configurations.
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	// Because we told our pipeline that the Viewport is dynamic, we must explicitly declare its coordinates right here.
	// This converts normalized shader space (-1 to 1) into physical screen pixels (0 to 1280x720).
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapChainExtent.width);
	viewport.height = static_cast<float>(m_swapChainExtent.height);
	viewport.minDepth = 0.0f; // Standard depth boundary settings
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport); // Push the viewport parameters onto the active command stream

	// Scissor defines a clipping bounding box. Anything drawn outside this boundary box gets thrown away by the GPU rasterizer.
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapChainExtent; // Match our entire screen width/height
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor); // Push the scissor clipping parameters onto the active command stream

	// THE MASTER DRAW TRIGGER CALL.
	// Parameters: 3 vertices, 1 instance profile, starting at vertex index 0, starting at instance index 0.
	// Because your vertex buffer input state is currently empty, this tells your vertex shader to execute exactly 
	// 3 times in parallel, allowing you to hardcode your milestone triangle coords directly inside the shader code!
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	// Closes the immediate dynamic rendering loop work block safely.
	vkCmdEndRendering(commandBuffer);

	// =========================================================================
	// STEP 6: HARDWARE BARRIER — TRANSITION CANVAS BACK TO PRESENTATION SOURCE
	// =========================================================================
	// Before your operating system window manager can display this frame on your monitor, 
	// the layout must be shifted away from "Color Attachment Optimal" to "Present Source KHR".
	VkImageMemoryBarrier barrierToPresent{};
	barrierToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrierToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Current state coming out of drawing
	barrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;           // Final state needed for display presentation
	barrierToPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrierToPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrierToPresent.image = m_swapChainImages[imageIndex];
	barrierToPresent.subresourceRange = barrierToDraw.subresourceRange;    // Copies our identical color channel masks from step 2

	barrierToPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // The GPU must finish writing its pixel values completely...
	barrierToPresent.dstAccessMask = 0;                                    // ...before this layout shift is allowed to occur.

	// Executes our final layout transition barrier script line.
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // Wait Stage: Do not touch the image until the color hardware finishes writing pixels
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // Block Stage: Perform layout shift at the absolute end of the hardware channel execution
		0, 0, nullptr, 0, nullptr, 1, &barrierToPresent
	);

	// Closes the script macro file file entirely. The command buffer is now fully recorded and locked for queue submission!
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

/**
 * @brief Orchestrates the cyclic CPU-GPU engine frame pump.
 * Coordinates application multi-frame tracking lanes using synchronized fences,
 * requests an available target canvas image from the window swapchain, wipes and records
 * the corresponding lane command buffer, submits operations to the hardware graphics queue,
 * and flags the presentation queue to display the final drawn pixels on the monitor.
 */
void Application::drawFrame() {
	// 1. Wait on the fence dedicated to THIS specific frame lane
	vkWaitForFences(m_logicalDevice, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(m_logicalDevice, 1, &m_inFlightFences[m_currentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(m_logicalDevice, m_swapChain, UINT64_MAX,
		m_imageAvailableSemaphores[m_currentFrame], // Index tracking
		VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);
	recordCommandBuffer(m_commandBuffers[imageIndex], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Route sync variables matching the current index lane
	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[imageIndex] };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Hand over the specific fence tracking this execution slot
	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(m_presentQueue, &presentInfo);

	// 2. CYCLE TO THE NEXT TIMING SLOT (0 -> 1 -> 0 -> 1...)
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

/**
 * @brief Allocates all concurrent low-level execution sync primitives on the hardware.
 * Resizes tracking arrays to match maximum concurrent frames in flight limits, spawning
 * binary GPU semaphores for cross-pipeline tracking, and initializes CPU-gating fences
 * configured with pre-signaled bits to prevent deadlocks during frame zero startup.
 */
void Application::createSyncObjects() {

	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(m_swapChainImages.size());
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	// 1. CONFIGURE SEMAPHORE BLUEPRINTS
	// Semaphores are used for GPU-to-GPU synchronization. They control the execution order 
	// of commands on the graphics card without talking back to the CPU.
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO; // Tag the structure type

	// 2. CONFIGURE FENCE BLUEPRINT
	// Fences are used for GPU-to-CPU synchronization. They allow your C++ code running on 
	// the CPU to pause and wait until the GPU hardware finishes a specific workload bundle.
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	// CRITICAL ENGINE OPTIMIZATION:
	// We add the VK_FENCE_CREATE_SIGNALED_BIT flag. This forces the fence to start out 
	// "Signaled" (open/unlocked). On the very first frame of your main loop, drawFrame() 
	// calls vkWaitForFences(). If the fence starts closed, your CPU will freeze forever 
	// waiting for frame zero to finish rendering before it even has a chance to submit it!
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	// 3. ALLOCATE ALL THREE OBJECTS ON THE HARDWARE
	// Ask your logical device to physically bake these three primitives into GPU memory.
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	for (size_t i = 0; i < m_swapChainImages.size(); i++) {
		if (vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render finished semaphores!");
		}
	}

	std::cout << "Engine Sync: Fences and Semaphores successfully backed on hardware." << std::endl;
}