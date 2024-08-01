#pragma once

//defines a GPU

#include <Wireframe/DesktopWindow.hpp>

#include <VkBootstrap.h>

namespace Wireframe::Device
{
	//defines a GPU create info
	struct GPU_CreateInfo
	{
		bool isDebug = true; //is it debug

		uint32_t vulkanMajorVersion = 1, //major version
			vulkanMinorVersion = 2; //minor version

		bool specific13FeaturesNeeded = false; //are specific 1.3 features needed
		bool specific12FeaturesNeeded = false; //are specific 1.2 features needed

		//features
		VkPhysicalDeviceVulkan13Features features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };

		const char* appName = "Wireframe App";
	};

	//defines a GPU
	struct GPU
	{
		bool isDebug = true; //is it debug

		VkInstance instance;// Vulkan library handle
		VkDebugUtilsMessengerEXT debug_messenger;// Vulkan debug output handle
		VkPhysicalDevice chosenGPU;// GPU chosen as the default device
		VkDevice device; // Vulkan device for commands

		VkQueue graphicsQueue;
		uint32_t graphicsQueueFamily;

		//creates GPU
		inline bool Create(GPU_CreateInfo& info, Window::DesktopWindow* window)
		{
			//if the required API doesn't support a feature set
			if (info.specific13FeaturesNeeded && info.vulkanMinorVersion < 3)
			{
				fmt::print("Wireframe Error: GPU || Create || Vulkan 1.3 features were required but the required version (Ver: {}) is not 3 or highter\n", info.vulkanMinorVersion);
				return false;
			}
			else if (info.specific12FeaturesNeeded && info.vulkanMinorVersion < 2)
			{
				fmt::print("Wireframe Error: GPU || Create || Vulkan 1.2 features were required but the required version (Ver: {}) is not 2 or highter\n", info.vulkanMinorVersion);
				return false;
			}

			isDebug = info.isDebug;

			//make the vulkan instance
			vkb::InstanceBuilder builder;
			auto inst_ret = builder.set_app_name(info.appName)
				.request_validation_layers(info.isDebug)
				.use_default_debug_messenger()
				.require_api_version(info.vulkanMajorVersion, info.vulkanMinorVersion, 0)
				.build();
			if (!inst_ret)
			{
				fmt::print("WIREFRAME FATAL ERROR: GPU || Init (Vulkan {}.{}) || Failed to create Vulkan instance!\n", info.vulkanMajorVersion, info.vulkanMinorVersion);
				return false;
			}

			vkb::Instance vkb_inst = inst_ret.value();

			//grab the instance 
			instance = vkb_inst.instance;
			debug_messenger = vkb_inst.debug_messenger;

			window->CreateSurface(instance);

			//use vkbootstrap to select a gpu. 
			//We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
			vkb::PhysicalDeviceSelector selector{ vkb_inst };
			selector.set_minimum_version(1, 3)
				.set_surface(window->_surface);
			
			if (info.specific12FeaturesNeeded)
				selector.set_required_features_12(info.features12);
			if (info.specific13FeaturesNeeded)
				selector.set_required_features_13(info.features13);
				
			auto sel_ret = selector.select();

			if (!sel_ret)
			{
				window->DestroySurface(instance);
				vkb::destroy_debug_utils_messenger(instance, debug_messenger);
				vkDestroyInstance(instance, nullptr);
				fmt::print("WIREFRAME FATAL ERROR: GPU || Init (Vulkan {}.{}) || Failed to create Vulkan GPU Devices!\n", info.vulkanMajorVersion, info.vulkanMinorVersion);
				return false;
			}
			vkb::PhysicalDevice physicalDevice = sel_ret.value();

			//create the final vulkan device
			vkb::DeviceBuilder deviceBuilder{ physicalDevice };

			vkb::Device vkbDevice = deviceBuilder.build().value();

			// Get the VkDevice handle used in the rest of a vulkan application
			device = vkbDevice.device;
			chosenGPU = physicalDevice.physical_device;
			//< init_device

			//> init_queue
				// use vkbootstrap to get a Graphics queue
			graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
			graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
			//< init_queue

			return true;
		}

		//destroys GPU
		inline void Destroy()
		{
			vkDestroyDevice(device, nullptr);
			vkb::destroy_debug_utils_messenger(instance, debug_messenger);
			vkDestroyInstance(instance, nullptr);
		}
	};
}