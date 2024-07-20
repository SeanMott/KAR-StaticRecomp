/*
Tests for the new 3.0 Bytes The Dust SDK renderer
*/

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <VkBootstrap.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <Wireframe/Game/Window.hpp>

#include <Wireframe/vk_types.h>
#include <Wireframe/vk_initializers.h>
#include <Wireframe/GPU.hpp>
#include <Wireframe/Swapchain.hpp>
#include <Wireframe/vk_images.h>

#include <BTDSTD/Vectors.hpp>

#include <chrono>
#include <thread>

//defines all the data for the frame
struct FrameData
{
	VkCommandPool _commandPool;
	VkCommandBuffer _mainCommandBuffer;

	VkSemaphore _swapchainSemaphore, _renderSemaphore;
	VkFence _renderFence;
};

constexpr unsigned int FRAME_OVERLAP = 2;

//the data for the frames
FrameData _frames[FRAME_OVERLAP];
int _frameNumber = 0; //the current frame number
//gets the current framme
FrameData& get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

//initalizes all the engine resources
inline bool Init_EngineResources(BTD::Application::Application& app, BTD::Window::Window& window, Wireframe::GPU::GPU& GPU,
	VmaAllocator& _allocator, Wireframe::Swapchain::DesktopSwapchain& swapchain)
{
	BTD::Application::ApplicationCreateInfo appInfo;
	appInfo.name = "Test Renderer"; //sets the app name
	appInfo.os = BTD::OS::GetOS(); //gets the OS we compile to
	appInfo.renderBackend = BTD::OS::GetRenderBackend(); //gets the render API based on what we compile to

	appInfo.isDebug = true; //is the app debug

	if (!app.Init(&appInfo))
		return false;

	//creates window
	BTD::Window::WindowCreateInfo windowInfo;
	windowInfo.size = { 1700 , 900 };
	windowInfo.title = "SDL Vulkan Renderer";

	if (!window.Create(&windowInfo, &app))
		return false;

	//initalize the GPU
	Wireframe::GPU::GPUCreateInfo GPUInfo;

	//vulkan 1.3 features
	GPUInfo.set_1_3_features = true;
	GPUInfo.features13.dynamicRendering = true;
	GPUInfo.features13.synchronization2 = true;

	//vulkan 1.2 features
	GPUInfo.set_1_2_features = true;
	GPUInfo.features12.bufferDeviceAddress = true;
	GPUInfo.features12.descriptorIndexing = true;

	if (!GPU.Create(&GPUInfo, &window, &app))
		return false;

	// initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = GPU.chosenGPU;
	allocatorInfo.device = GPU.device;
	allocatorInfo.instance = GPU.instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	
	VmaVulkanFunctions vma_vulkan_func{};
	vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
	vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
	vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
	vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
	vma_vulkan_func.vkCreateImage = vkCreateImage;
	vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
	vma_vulkan_func.vkDestroyImage = vkDestroyImage;
	vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vma_vulkan_func.vkFreeMemory = vkFreeMemory;
	vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vma_vulkan_func.vkMapMemory = vkMapMemory;
	vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
	vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer;
	vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	allocatorInfo.pVulkanFunctions = &vma_vulkan_func;
	vmaCreateAllocator(&allocatorInfo, &_allocator);

	//creates the swapchain
	Wireframe::Swapchain::DesktopSwapchainCreateInfo swapchainInfo;

	if (!swapchain.Create(&swapchainInfo, &GPU, &window))
		return false;

	return true;
}

//cleans up all the engine resources
inline void Shutdown_EngineResources(BTD::Application::Application& app, BTD::Window::Window& window, Wireframe::GPU::GPU& GPU,
	VmaAllocator& _allocator, Wireframe::Swapchain::DesktopSwapchain& swapchain)
{
	//destroys swapchain
	swapchain.Destroy(&GPU);

	//destroy surface
	window.DestroySurface(GPU.instance);

	//clean up allocator
	vmaDestroyAllocator(_allocator);

	//destroy gpu
	GPU.Destroy();

	window.Destroy();
	app.Shutdown();
}

//initalizes sync objects and command buffers
inline bool Init_SyncAndRenderResources(Wireframe::GPU::GPU& GPU)
{
	VkDevice device = GPU.device;

	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(GPU.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

		VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
	}

	//create syncronization structures
	//one fence to control when the gpu has finished rendering the frame,
	//and 2 semaphores to syncronize rendering with swapchain
	//we want the fence to start signalled so we can wait on it on the first frame
	VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &_frames[i]._swapchainSemaphore));
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));
	}

	return true;
}

//cleans sync objects and command buffers
inline void Shutdown_SyncAndRenderResources(VkDevice& device)
{
	//destroy sync structures and command objects
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		//destroy command pool and buffers
		vkDestroyCommandPool(device, _frames[i]._commandPool, nullptr);

		//destroy sync objects
		vkDestroyFence(device, _frames[i]._renderFence, nullptr);
		vkDestroySemaphore(device, _frames[i]._renderSemaphore, nullptr);
		vkDestroySemaphore(device, _frames[i]._swapchainSemaphore, nullptr);
	}
}

//allocates render targets
inline bool Init_AllocateRenderTargets(vkutil::AllocatedImage& renderImage, const VkExtent2D& windowSize, VmaAllocator& _allocator, VkDevice& device)
{
	//draw image size will match the window
	VkExtent3D drawImageExtent = {
		windowSize.width,
		windowSize.height,
		1
	};

	//hardcoding the draw format to 32 bit float
	renderImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	renderImage.imageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = vkinit::image_create_info(renderImage.imageFormat, drawImageUsages, drawImageExtent);

	//for the draw image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &renderImage.image, &renderImage.allocation, nullptr);

	//build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo rview_info = vkinit::imageview_create_info(renderImage.imageFormat, renderImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(device, &rview_info, nullptr, &renderImage.imageView));

	return true;
}

//cleans up render targets
inline void Shutdown_AllocateRenderTargets(vkutil::AllocatedImage& renderImage, VmaAllocator& _allocator, VkDevice& device)
{
	//cleans up the render target
	vkDestroyImageView(device, renderImage.imageView, nullptr);
	vmaDestroyImage(_allocator, renderImage.image, renderImage.allocation);
}

//draws the background
inline void RenderOperation_Background(VkCommandBuffer cmd, vkutil::AllocatedImage& renderImage)
{
	//make a clear-color from frame number. This will flash with a 120 frame period.
	VkClearColorValue clearValue;
	float flash = std::abs(std::sin(_frameNumber / 120.f));
	clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

	VkImageSubresourceRange clearRange = vkinit::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

	//clear image
	vkCmdClearColorImage(cmd, renderImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
}

//draws a frame
inline bool DrawFrame(Wireframe::GPU::GPU& GPU, Wireframe::Swapchain::DesktopSwapchain& swapchain, vkutil::AllocatedImage& renderImage,
	bool& swapchainNeedsToBeResized)
{
	VkDevice device = GPU.device;

	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(device, 1, &get_current_frame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(device, 1, &get_current_frame()._renderFence));

	//request image from the swapchain
	uint32_t swapchainImageIndex;
	VkResult r = vkAcquireNextImageKHR(device, swapchain.swapchain, 1000000000, get_current_frame()._swapchainSemaphore, nullptr, &swapchainImageIndex);
	if (r == VkResult::VK_SUBOPTIMAL_KHR || r == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
	{
		swapchainNeedsToBeResized = true;
		return false;
	}

	//naming it cmd for shorter writing
	VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkExtent2D renderSize;
	renderSize.width = renderImage.imageExtent.width;
	renderSize.height = renderImage.imageExtent.height;

	//start the command buffer recording
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// transition our main draw image into general layout so we can write into it
	// we will overwrite it all so we dont care about what was the older layout
	vkutil::transition_image(cmd, renderImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	//render the background
	RenderOperation_Background(cmd, renderImage);

	//run the indirect command buffer mesh renderer

	//transition the draw image and the swapchain image into their correct transfer layouts
	vkutil::transition_image(cmd, renderImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	vkutil::transition_image(cmd, swapchain.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// execute a copy from the draw image into the swapchain
	vkutil::copy_image_to_image(cmd, renderImage.image, swapchain.swapchainImages[swapchainImageIndex], renderSize, swapchain.swapchainExtent);

	// set swapchain image layout to Present so we can show it on the screen
	vkutil::transition_image(cmd, swapchain.swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue. 
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished
	VkCommandBufferSubmitInfo cmdinfo = vkinit::command_buffer_submit_info(cmd);

	VkSemaphoreSubmitInfo waitInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame()._swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo = vkinit::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, get_current_frame()._renderSemaphore);

	VkSubmitInfo2 submit = vkinit::submit_info(&cmdinfo, &signalInfo, &waitInfo);

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(GPU.graphicsQueue, 1, &submit, get_current_frame()._renderFence));

	//prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that, 
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &swapchain.swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	r = vkQueuePresentKHR(GPU.graphicsQueue, &presentInfo);
	if (r == VkResult::VK_SUBOPTIMAL_KHR || r == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
	{
		swapchainNeedsToBeResized = true;
		return false;
	}

	//increase the number of frames drawn
	_frameNumber++;

	return true;
}

//resiszes the swapchain
static inline void ResizeSwapchain(Wireframe::GPU::GPU* GPU, BTD::Window::Window* window,
	Wireframe::Swapchain::DesktopSwapchain* swapchain)
{
	vkDeviceWaitIdle(GPU->device); //waits for GPU to finish

	swapchain->Destroy(GPU);

	int w, h;
	SDL_GetWindowSize(window->window, &w, &h);
	window->windowSize = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };

	Wireframe::Swapchain::DesktopSwapchainCreateInfo swapchainInfo;
	swapchain->Create(&swapchainInfo, GPU, window);
}

//entry point
int main()
{
	//---init

	//initalize engine resources
	BTD::Application::Application app; BTD::Window::Window window; Wireframe::GPU::GPU GPU;
	VmaAllocator _allocator; Wireframe::Swapchain::DesktopSwapchain swapchain; bool swapchainNeedsToBeResized = false;
	if (!Init_EngineResources(app, window, GPU, _allocator, swapchain))
	{
		Shutdown_EngineResources(app, window, GPU, _allocator, swapchain);
		getchar();
		return -1;
	}

	//allocate render targets
	vkutil::AllocatedImage renderImage;
	if (!Init_AllocateRenderTargets(renderImage, window.windowSize, _allocator, GPU.device))
	{
		Shutdown_AllocateRenderTargets(renderImage, _allocator, GPU.device);
		Shutdown_EngineResources(app, window, GPU, _allocator, swapchain);
		getchar();
		return -1;
	}

	//create sync objects and render resources
	if (!Init_SyncAndRenderResources(GPU))
	{
		Shutdown_SyncAndRenderResources(GPU.device);
		Shutdown_AllocateRenderTargets(renderImage, _allocator, GPU.device);
		Shutdown_EngineResources(app, window, GPU, _allocator, swapchain);
		getchar();
		return -1;
	}

	//---game loop
	while (window.isRunning)
	{
		//---delta time

		//---input
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) //polls input
		{
			// close the window when user alt-f4s or clicks the X button
			if (e.type == SDL_EVENT_QUIT)
			{
				window.isRunning = false;
				break;
			}

			//minimized the window
			if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
				window.isMinimized = true;

			//unmimized the window
			if (e.type == SDL_EVENT_WINDOW_RESTORED)
				window.isMinimized = false;
		}

		//---update

		//---draw

		// do not draw if we are minimized
		if (window.isMinimized)
		{
			// throttle the speed to avoid the endless spinning
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		//performs a frame render and draw
		if (!DrawFrame(GPU, swapchain, renderImage, swapchainNeedsToBeResized))
		{
			//remake the swapchain
			if (swapchainNeedsToBeResized)
			{
				ResizeSwapchain(&GPU, &window, &swapchain);
				swapchainNeedsToBeResized = false;
			}

		}
	}
	vkDeviceWaitIdle(GPU.device); //waits for GPU to finish

	//---clean up

	//clean up sync objects and command buffers
	Shutdown_SyncAndRenderResources(GPU.device);

	//cleans up the render targets
	Shutdown_AllocateRenderTargets(renderImage, _allocator, GPU.device);

	//clean up engine resources
	Shutdown_EngineResources(app, window, GPU, _allocator, swapchain);

	getchar();
	return 0;
}