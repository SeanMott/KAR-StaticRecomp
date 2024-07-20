#pragma once

//defines vectors

#include <vulkan/vulkan.h>

namespace BTD::Math
{
	//defines a uint32 vec2
	struct UIVec2
	{
		union
		{
			uint32_t x;
			uint32_t width;
		};

		union
		{
			uint32_t y;
			uint32_t width;
		};

		//converts a vkExtent to uinvec2
		inline VkExtent2D Extent2D() const { return { x, y }; }

		//converts a uinvec2 to vkExtent
	};
}