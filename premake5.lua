ENGINE_NAME = "PurEngine"

dofile ("premake/amd_premake_util.lua")
dofile ("premake/utils.lua")

-- Workspace (Solution)
workspace (ENGINE_NAME)
    configurations { "Debug", "Release", "Final" }
    platforms { "x64" }
    location "build"
    filename (ENGINE_NAME .. _AMD_VS_SUFFIX)
    startproject ("Demo")

    filter "platforms:x64"
        system "Windows"
        architecture "x64"

dofile("External/premake5.lua")
dofile("Engine/premake5.lua")
dofile("Cooker/premake5.lua")
dofile("Shaders/premake5.lua")
dofile("Content/premake5.lua")
dofile("Demo/premake5.lua")