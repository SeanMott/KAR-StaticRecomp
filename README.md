# KAR-StaticRecomp
A PC port of Kirby Air Ride North America, code name Projekt Swerve. Based on code from [OG KAR decomp](https://github.com/doldecomp/kar), and a [DTK decomp](https://github.com/SeanMott/PPC-KAR-HP-Decomp).

NO ASSETS ARE PROVIDED IN THIS REPO, YOU MUST PROVIDE YOUR OWN ROM!!!

# Platforms

## Windows

## Mac

## Linux

# Dependicies

[Bytes The Dust Devkit]() for making video games and other applications with C++. Mainly [BTDSTD]() and [Wireframe]()

[Valve's Open source Networking library](https://github.com/ValveSoftware/GameNetworkingSockets) since we want networking unbound by any stores.

[Valve's open source sound library](https://github.com/ValveSoftware/steam-audio) since we don't want to be limited to game cube audio.

[Jolt](https://github.com/jrouwe/JoltPhysics) is used for physics. As the exacte physics code couldn't be extracted verbatim. So we substitute and as more tweaking and asm is recovered. The project will be updated.

[Recast Navigation](https://github.com/recastnavigation/recastnavigation) is used for nav meshes of the AIs.

## Game Cube and Wii Deps

Compiling to native GC or Wii is not supported at the moment. If it does become a supported platform binutils and Devkit Pro will be required.

## Other Platforms

Maybe, PC is the main target for right now.