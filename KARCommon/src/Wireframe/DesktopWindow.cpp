#include <Wireframe/DesktopWindow.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>

//creates a window
bool Wireframe::Window::DesktopWindow::Create(const DesktopWindow_CreateInfo& info)
{
	_windowExtent = info.size;

	// We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

	_window = SDL_CreateWindow(
		info.title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowExtent.width,
		_windowExtent.height,
		window_flags
	);
	if (!_window)
	{
		fmt::print(fmt::fg(fmt::color::red), "WIREFRAME FATAL ERROR: Desktop Window || Create || Failed to create a Window!\n");
		return false;
	}

	return true;
}

//destroys a window
void Wireframe::Window::DesktopWindow::Destroy()
{
	SDL_DestroyWindow(_window);
}

//creates a surface
void Wireframe::Window::DesktopWindow::CreateSurface(VkInstance& instance)
{
	SDL_Vulkan_CreateSurface(_window, instance, &_surface);
}

//destroys a surface
void Wireframe::Window::DesktopWindow::DestroySurface(VkInstance& instance)
{
	vkDestroySurfaceKHR(instance, _surface, nullptr);
}
