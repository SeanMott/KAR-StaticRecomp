#pragma once

//defines a window

#include <Wireframe/Game/Application.hpp>

#include <volk.h>
#include <vulkan/vulkan.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace BTD::Window
{
	//defines a window create info
	struct WindowCreateInfo
	{
		VkExtent2D size = { 1700 , 900 };

		const char* title = "BTD Window";
	};

	//defines a window
	struct Window
	{
		bool isRunning = false;
		bool isMinimized = false;

		VkExtent2D windowSize = { 1700 , 900 };

		VkSurfaceKHR surface = VK_NULL_HANDLE; // Vulkan window surface
		SDL_Window* window = nullptr;
		const char* title = "BTD Window";

		//creates window
		inline bool Create(const WindowCreateInfo* info, const Application::Application* app)
		{
			SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

			window = SDL_CreateWindow(
				info->title,
				info->size.width,
				info->size.height,
				window_flags);
			if (!window)
			{
				fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "Failed to create a Window titled: \"{}\"\n", info->title);
				return -1;
			}
			windowSize = info->size;
			title = info->title;

			isRunning = true;
			return true;
		}

		//destroys window
		inline void Destroy()
		{
			if (!window)
				return;

			if(surface != VK_NULL_HANDLE)
				fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "You never clean up the Vulkan Surface for the Window titled: \"{}\"\n", title);

			isRunning = false; isMinimized = false;
			SDL_DestroyWindow(window); window = nullptr;
		}

		//creates a surface
		inline VkSurfaceKHR& CreateSurface(VkInstance instance)
		{
			if (SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface) == -1)
				fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "Failed to create a Window Surface using Window titled: \"{}\"\n", title);

			return surface;
		}

		//destroys a surface
		inline void DestroySurface(VkInstance instance)
		{
			if (!window || surface == VK_NULL_HANDLE)
				return;

			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = VK_NULL_HANDLE;
		}
	};
}