RENDERER_NAME = "Renderer"

project (RENDERER_NAME)
    kind "StaticLib"
    language "C++"
    location "build"
    filename (RENDERER_NAME .. _AMD_VS_SUFFIX)
    uuid "3F3B938C-ABC6-0051-B4D7-834520E25C52"
    targetdir "../../bin"
    objdir "build/%{_AMD_SAMPLE_DIR_LAYOUT}"
    warnings "Extra"
    floatingpoint "Fast"
    dependson { CORE_NAME }

    files { "source/**.h", "source/**.cpp" }
    links { CAPNPROTO_NAME, TYPES_NAME, "d3dcompiler", "dxguid", "d3d12", "dxgi", "WinPixEventRuntime" }
    linkoptions { "-IGNORE:4006" }
    disablewarnings { "4238" }

    libdirs { "lib" }
    includedirs { "../%{CORE_NAME}/source", "../%{TYPES_NAME}/source", "../../External/%{CAPNPROTO_NAME}/src", "include" }

    defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX" }

    filter "configurations:Debug"
        defines { "WIN32", "_DEBUG", "DEBUG", "_WINDOWS" }
        flags { "FatalWarnings"}
        symbols "On"
        targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

    filter "configurations:Release"
        defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "RELEASE" }
        flags { "LinkTimeOptimization", "FatalWarnings"}
        symbols "On"
        targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
        optimize "On"

    filter "configurations:Final"
        defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "FINAL" }
        flags { "LinkTimeOptimization", "FatalWarnings"}
        symbols "On"
        targetsuffix ("_Final" .. _AMD_VS_SUFFIX)
        optimize "On"

    postbuildcommands { "copy lib/WinPixEventRuntime.dll ../../bin/" }