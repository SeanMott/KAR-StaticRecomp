#pragma once

//defines application stuff

#include <volk.h>
#include <SDL3/SDL.h>

#include <BTDSTD/Logging.hpp>
#include <BTDSTD/OS.hpp>

namespace BTD::Application
{
	//defines a application create info
	struct ApplicationCreateInfo
	{
		bool isDebug = true;

		BTD::OS::OS os = BTD::OS::OS::Desktop_Windows;
		BTD::OS::RenderBackend renderBackend = BTD::OS::RenderBackend::Vulkan;

		const char* name = "BTD App";
	};

	//defines a application
	struct Application
	{
		bool isDebug = true;

		BTD::OS::OS os = BTD::OS::OS::Desktop_Windows;
		BTD::OS::RenderBackend renderBackend = BTD::OS::RenderBackend::Vulkan;

		const char* name = "BTD App";

		//initalize the app and it's settings
		inline bool Init(const ApplicationCreateInfo* info)
		{
			isDebug = info->isDebug;
			name = info->name;

			os = info->os;
			renderBackend = info->renderBackend;

			//initalize Volk
			if (volkInitialize() != VK_SUCCESS)
			{
				fmt::print(fg(fmt::color::dark_red), "Volk Failed to init!\n");
				return false;
			}

			//initalize SDL and tell it we need the video
			if (SDL_Init(SDL_INIT_VIDEO) == -1)
			{
				fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "SDL FATAL ERROR: {}", SDL_GetError());
				return false;
			}
			fmt::print(fg(fmt::color::cyan), "SDL initalized.\n");

			return true;
		}

		//shutsdown the app
		inline void Shutdown()
		{
			SDL_Quit(); //shutsdown SDL
			fmt::print(fg(fmt::color::cyan), "SDL shutdown.\n");
		}
	};
}