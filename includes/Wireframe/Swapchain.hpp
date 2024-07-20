#pragma once

//defines a swapchain

#include <Wireframe/GPU.hpp>

namespace Wireframe::Swapchain
{
	//defines a desktop swapchain create info
	struct DesktopSwapchainCreateInfo
	{

	};

	//defines a desktop swapchain
	struct DesktopSwapchain
	{
		VkSwapchainKHR swapchain;
		VkFormat swapchainImageFormat;

		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;
		VkExtent2D swapchainExtent;

		//creates a swapchain (this is a fresh from scratch one)
		inline bool Create(const DesktopSwapchainCreateInfo* info, const GPU::GPU* GPU, const BTD::Window::Window* window)
		{
			vkb::SwapchainBuilder swapchainBuilder{ GPU->chosenGPU, GPU->device, window->surface };

			swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

			vkb::Swapchain vkbSwapchain = swapchainBuilder
				//.use_default_format_selection()
				.set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
				//use vsync present mode
				.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
				.set_desired_extent(window->windowSize.width, window->windowSize.height)
				.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
				.build()
				.value();

			swapchainExtent = vkbSwapchain.extent;
			//store swapchain and its related images
			swapchain = vkbSwapchain.swapchain;
			swapchainImages = vkbSwapchain.get_images().value();
			swapchainImageViews = vkbSwapchain.get_image_views().value();

			return true;
		}

		//destroys a swapchain (destroys EVERYTHING)
		inline void Destroy(const GPU::GPU* GPU)
		{
			vkDestroySwapchainKHR(GPU->device, swapchain, nullptr);

			// destroy swapchain resources
			for (int i = 0; i < swapchainImageViews.size(); i++) {

				vkDestroyImageView(GPU->device, swapchainImageViews[i], nullptr);
			}
		}

		//light create a swapchain (used for recreation)

		//light destroy a swapchain (used for recreation)

		//remakes a swapchain

	};

	//defines a VR swapchain create info using OpenXR

	//defines a VR swapchain using OpenXR
}