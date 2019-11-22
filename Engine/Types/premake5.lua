TYPES_NAME = "Types"

print("-- Compiling CapNProto for Engine.Types --")
-- CompileCapNProto(workDir, matchPattern, capnpPath, compilerName) -- All paths need to be relative to supplied workDir!
CompileCapNProto("source", "../../../Prototypes/**.capnp", "..\\..\\..\\External\\capnproto\\bin\\capnp.exe", "c++")

project (TYPES_NAME)
    kind "StaticLib"
    language "C++"
    location "build"
    filename (TYPES_NAME .. _AMD_VS_SUFFIX)
    uuid "3F3B938C-ABC6-0051-B4D7-834520E25C50"
    targetdir "../../bin"
    objdir "build/%{_AMD_SAMPLE_DIR_LAYOUT}"
    warnings "Off"
    floatingpoint "Fast"
    dependson { CAPNPROTO_NAME }

    files { "source/**.h", "source/**.cpp" }
    links { CAPNPROTO_NAME }
    defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX" }

    includedirs { "../../External/%{CAPNPROTO_NAME}/src" }

    filter "configurations:Debug"
        defines { "WIN32", "_DEBUG", "DEBUG", "_WINDOWS" }
        flags { }
        symbols "On"
        targetsuffix ("_Debug" .. _AMD_VS_SUFFIX)

    filter "configurations:Release"
        defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "RELEASE" }
        flags { "LinkTimeOptimization" }
        symbols "On"
        targetsuffix ("_Release" .. _AMD_VS_SUFFIX)
        optimize "On"

    filter "configurations:Final"
        defines { "WIN32", "NDEBUG", "PROFILE", "_WINDOWS", "FINAL" }
        flags { "LinkTimeOptimization" }
        symbols "On"
        targetsuffix ("_Final" .. _AMD_VS_SUFFIX)
        optimize "On"