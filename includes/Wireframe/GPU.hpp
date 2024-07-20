#pragma once

//defines a GPU

#include <VkBootstrap.h>

#include <Wireframe/Game/Window.hpp>

namespace Wireframe::GPU
{
	//defines a create info
	struct GPUCreateInfo
	{
		//will specific features be set
		bool set_1_2_features = false;
		bool set_1_3_features = false;

		//features
		VkPhysicalDeviceVulkan13Features features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	};

	//defines a GPU
	struct GPU
	{
		VkInstance instance;// Vulkan library handle
		VkDebugUtilsMessengerEXT debugMessenger;// Vulkan debug output handle
		
		VkPhysicalDevice chosenGPU;// GPU chosen as the default device
		VkDevice device; // Vulkan device for commands

		//graphics queue
		VkQueue graphicsQueue;
		uint32_t graphicsQueueFamily;

		//present queue

		//creates a GPU
		inline bool Create(GPUCreateInfo* info, BTD::Window::Window* window, const BTD::Application::Application* app)
		{
			

			vkb::InstanceBuilder builder;

			//make the vulkan instance, with basic debug features
			auto inst_ret = builder.set_app_name(app->name)
				.request_validation_layers(app->isDebug)
				.use_default_debug_messenger()
				.require_api_version(1, 3, 0)
				.build();

			vkb::Instance vkb_inst = inst_ret.value();

			//grab the instance 
			instance = vkb_inst.instance;
			debugMessenger = vkb_inst.debug_messenger;
			volkLoadInstance(instance);

			window->CreateSurface(instance);

			//use vkbootstrap to select a gpu. 
			//We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
			vkb::PhysicalDeviceSelector selector{ vkb_inst };
			selector.set_minimum_version(1, 3).set_surface(window->surface);

			//sets features
			if (info->set_1_2_features)
				selector.set_required_features_12(info->features12);
			if (info->set_1_3_features)
				selector.set_required_features_13(info->features13);

			//gets GPU
			vkb::PhysicalDevice physicalDevice = selector.select().value();

			//create the final vulkan device
			vkb::DeviceBuilder deviceBuilder{ physicalDevice };

			vkb::Device vkbDevice = deviceBuilder.build().value();

			// Get the VkDevice handle used in the rest of a vulkan application
			device = vkbDevice.device;
			chosenGPU = physicalDevice.physical_device;
			volkLoadDevice(device);

			// use vkbootstrap to get a Graphics queue
			graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
			graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

			return true;
		}

		//destroys a GPU
		inline void Destroy()
		{
			if (instance == VK_NULL_HANDLE)
				return;

			if (device != VK_NULL_HANDLE)
				vkDestroyDevice(device, nullptr);

			vkb::destroy_debug_utils_messenger(instance, debugMessenger);
			vkDestroyInstance(instance, nullptr);
		}
	};
}