#gets Vulkan libraries

import GitOperations

#generate the Premake string for adding the include dir
def GeneratePremake_IncludePath():
    return """"Venders/VKBootstrap/src",
    "Venders/VMA/include","""



#clones Vulkan libraries
def GetDep_VulkanLibraries():
    GitOperations.GitClone("https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git", "VMA")
    GitOperations.GitClone("https://github.com/charles-lunarg/vk-bootstrap.git", "VKBootstrap", "main")