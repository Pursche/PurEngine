CAPNPROTO_NAME = "CapNProto"

project (CAPNPROTO_NAME)
    kind "StaticLib"
    language "C++"
    location "build"
    filename (CAPNPROTO_NAME .. _AMD_VS_SUFFIX)
    uuid "3F3B938C-ABC6-0051-B4D7-834520E25C57"
    targetdir "../../bin"
    objdir "build/%{_AMD_SAMPLE_DIR_LAYOUT}"
    warnings "Off"
    floatingpoint "Fast"

    files { "src/**.h", "src/**.cpp", "src/**.c++" }
    defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX" }

    includedirs { "src" }

    filter "configurations:Debug"
        defines { "WIN32", "_DEBUG", "DEBUG", "_WINDOWS" }
        symbols "On"
        targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

    filter "configurations:Release"
        defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "RELEASE" }
        flags { "LinkTimeOptimization"}
        symbols "On"
        targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
        optimize "On"

    filter "configurations:Final"
        defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "FINAL" }
        flags { "LinkTimeOptimization"}
        symbols "On"
        targetsuffix ("_Final" .. _AMD_VS_SUFFIX)
        optimize "On"