#pragma once

//defines various OS and render targets

namespace BTD::OS
{
	//defines the OS
	enum class OS
	{
		Desktop_Windows = 0,

		Desktop_Mac_Intel,
		Desktop_Mac_MSeries,

		Desktop_Linux,

		Web,

		Mobile_Android,
		Mobile_IOS,

		Count
	};

	//gets the OS based on what we compile to
	inline OS GetOS()
	{
#ifdef Window_Build
		return OS::Desktop_Windows;

#else
		return OS::Count;
#endif
	}

	//defines the render target
	enum class RenderBackend
	{
		Vulkan = 0, //both compute and graphics will be used, or just genaric

		//specifics graphics will be used

		//specifices compute will be used

		Count
	};

	//gets the render backend based on what we compile to
	inline RenderBackend GetRenderBackend()
	{
#ifdef Window_Build
		return RenderBackend::Vulkan;

#else
		return RenderBackend::Count;
#endif
	}
}