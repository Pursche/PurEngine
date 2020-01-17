DEMO_NAME = "Demo"

project (DEMO_NAME)
    kind "WindowedApp"
    language "C++"
    location "build"
    filename (DEMO_NAME .. _AMD_VS_SUFFIX)
    uuid "3F3B938C-ABC6-0051-B4D7-834520E25C54"
    targetdir "../bin"
    objdir "build/%{_AMD_SAMPLE_DIR_LAYOUT}"
    warnings "Extra"
    floatingpoint "Fast"
    dependson { CORE_NAME, RENDERER_NAME, SHADERS_NAME }
    
    files { "source/**.h", "source/**.cpp" }
    links { CORE_NAME, RENDERER_NAME }
    includedirs { "../Engine/%{CORE_NAME}/source", "../Engine/%{RENDERER_NAME}/source"}

    disablewarnings { "4307" } -- Disable C4307: "integral constant overflow" because our compiletime string hashing uses overflow as a feature and we'll want to const the hashed strings

    defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX" }

    filter "configurations:Debug"
        defines { "WIN32", "_DEBUG", "DEBUG", "_WINDOWS" }
        flags { "FatalWarnings" }
        entrypoint "WinMainCRTStartup"
        symbols "On"
        targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

    filter "configurations:Release"
        defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "RELEASE" }
        flags { "LinkTimeOptimization", "FatalWarnings" }
        entrypoint "WinMainCRTStartup"
        symbols "On"
        targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
        optimize "On"

    filter "configurations:Final"
        defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "FINAL" }
        flags { "LinkTimeOptimization", "FatalWarnings" }
        entrypoint "WinMainCRTStartup"
        symbols "On"
        targetsuffix ("_Final" .. _AMD_VS_SUFFIX)
        optimize "On"
