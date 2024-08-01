
#include <string>

#include <vk_engine.h>

#include <filesystem>

//entry poiny
int main()
{
	//check if extracted ROM content folder exists
	const bool ROMFolderWasFound = std::filesystem::exists("ROMContent");

	//check if Mod content folder exists

	//load application
	VulkanEngine engine;

	engine.init(ROMFolderWasFound);

	engine.run();

	engine.cleanup();

	getchar();
	return 0;
}