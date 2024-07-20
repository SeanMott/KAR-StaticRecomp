"""
Sets up the project and gathers all the dependices and compiles the ones that are needed
"""

import subprocess
import os

#get repos
SDL_GIT_REPO_LINK = "https://github.com/libsdl-org/SDL.git"
FMT_GIT_REPO_LINK = "https://github.com/fmtlib/fmt.git"
GLM_GIT_REPO_LINK = "https://github.com/g-truc/glm.git"
YAML_CPP_GIT_REPO_LINK = "https://github.com/jbeder/yaml-cpp.git"

VULKAN_HEADERS_GIT_REPO_LINK = "https://github.com/KhronosGroup/Vulkan-Headers.git"
VMA_GIT_REPO_LINK = "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git"
VOLK_GIT_REPO_LINK = "https://github.com/zeux/volk.git"
BOOTSTRAPPER_GIT_REPO_LINK = "https://github.com/charles-lunarg/vk-bootstrap.git"

#if the folder doesn't exist, we download it
def GetIfNotThere(URL, outputDir):
    if not os.path.exists(outputDir):
        subprocess.run(["git", "clone", URL, outputDir],
        shell=True)

#gets SDL 3
GetIfNotThere(SDL_GIT_REPO_LINK, "Venders/SDL")

#builds SDL 3
subprocess.run(["cmake", "-S", "Venders/SDL", "-B", "Venders/SDL/Build"],
        shell=True)
subprocess.run(["msbuild", "Venders/SDL/Build/SDL3.sln", "-maxCpuCount:4", "/property:Configuration=Release"],
        shell=True)
subprocess.run(["msbuild", "Venders/SDL/Build/SDL3.sln", "-maxCpuCount:4", "/property:Configuration=MinSizeRel"],
        shell=True)
subprocess.run(["msbuild", "Venders/SDL/Build/SDL3.sln", "-maxCpuCount:4", "/property:Configuration=RelWithDebInfo"],
        shell=True)
           
#gets FMT
GetIfNotThere(FMT_GIT_REPO_LINK, "Venders/FMT")

#gets GLM
GetIfNotThere(GLM_GIT_REPO_LINK, "Venders/GLM")

#gets Vulkan headers
GetIfNotThere(VULKAN_HEADERS_GIT_REPO_LINK, "Venders/VulkanHeaders")

#gets VMA
GetIfNotThere(VMA_GIT_REPO_LINK, "Venders/VMA")

#gets Volk
GetIfNotThere(VOLK_GIT_REPO_LINK, "Venders/Volk")

#gets Bootstrapper
GetIfNotThere(BOOTSTRAPPER_GIT_REPO_LINK, "Venders/VKBootstrap")

#generate Premake file
premakeCode = """
workspace "BTDSDK_3.0"
architecture "x64"
startproject "TestRenderer"

configurations
{
    "Debug",
    "Release",
    "Dist"
}

project "TestRenderer"
kind "ConsoleApp"
language "C++"

targetdir (\"bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}-%{cfg.startproject}/%{prj.name}\")
objdir (\"bin-obj/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}-%{cfg.startproject}/%{prj.name}\")


files 
{
    ---base code
    "includes/**.h",
    "src/**.c",
    "includes/**.hpp",
    "src/**.cpp",

    --volk
    "Venders/Volk/volk.c",
    "Venders/Volk/volk.h",

    ---vulkan bootstrapper
    "Venders/VKBootstrap/src/**.cpp",
    "Venders/VKBootstrap/src/**.h"

    ---imgui
}

includedirs
{
    "includes",

    "Venders/SDL/include",
    "Venders/FMT/include",
    "Venders/GLM",

    "Venders/VKBootstrap/src",
    "Venders/VMA/include",
    "Venders/Volk",
    "Venders/VulkanHeaders/include"
}

links
{
    
}

defines
{
    "GLM_FORCE_RADIANS",
    "GLM_FORCE_DEPTH_ZERO_TO_ONE",
    "GLM_ENABLE_EXPERIMENTAL",
    "VK_NO_PROTOTYPES"
}

flags
{
    "NoRuntimeChecks",
    "MultiProcessorCompile"
}

buildoptions { "/utf-8" } --used for fmt

--platforms
filter "system:windows"
    cppdialect "C++20"
    staticruntime "On"
    systemversion "latest"

    defines
    {
        "Window_Build",
        "Desktop_Build",
        "VK_USE_PLATFORM_WIN32_KHR"
    }

filter "system:linux"
    cppdialect "C++20"
    staticruntime "On"
    systemversion "latest"

    defines
    {
        "Linux_Build",
        "Desktop_Build",
        "VK_USE_PLATFORM_XLIB_KHR"
    }

    filter "system:mac"
    cppdialect "C++20"
    staticruntime "On"
    systemversion "latest"

    defines
    {
        "MacOS_Build",
        "Desktop_Build",
        "VK_USE_PLATFORM_MACOS_MVK"
    }

--configs
filter "configurations:Debug"
    defines "BTD_DEBUG"
    symbols "On"

    links
    {
        "Venders/SDL/Build/RelWithDebInfo/SDL3.lib"
    }

filter "configurations:Release"
    defines "BTD_RELEASE"
    optimize "On"

    flags
    {
        
    }

    links
    {
       "Venders/SDL/Build/Release/SDL3.lib"
    }

filter "configurations:Dist"
    defines "BTD_DIST"
    optimize "On"

    defines
    {
        "NDEBUG"
    }

    flags
    {
       "LinkTimeOptimization"
    }

    links
    {
       "Venders/SDL/Build/MinSizeRel/SDL3.lib"
    }
"""

file = open("Premake5.lua", "w")
file.write(premakeCode)
file.close()

subprocess.run(["GenProjects.bat"],
        shell=True)

print("\n\n\n------Project is all ready to go :3-------")