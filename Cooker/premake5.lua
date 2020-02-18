COOKER_NAME = "Cooker"

print("-- Compiling CapNProto for Cooker --")
-- CompileCapNProto(workDir, matchPattern, capnpPath, compilerName) -- All paths need to be relative to supplied workDir!
CompileCapNProto("source", "../../Prototypes/**.capnp", "..\\..\\External\\capnproto\\bin\\capnp.exe", "csharp")

project (COOKER_NAME)
    kind "WindowedApp"
    architecture "x32"
    language "C#"
    location "build"
    filename (COOKER_NAME)
    uuid "3F3B938C-ABC6-0051-B4D7-834520E25C55"
    targetdir "../bin"
    objdir "build/%{_AMD_SAMPLE_DIR_LAYOUT}"
    warnings "Extra"
    floatingpoint "Fast"
    clr "Unsafe"
    
    nuget { "DynamicLanguageRuntime:1.2.2", "NeoLua:1.3.11", "Capnp.Net.Runtime:1.1.112", "Vortice.DXC:1.5.0" }
    files { "source/**.cs", "source/**.hlsl" }
    links { "System","System.Core", "Microsoft.CSharp","System.Runtime.Serialization","System.ComponentModel.DataAnnotations", "Microsoft.Scripting", "Neo.IronLua", "System.Drawing", "System.Drawing.Imaging", "System.XML" }
    
    filter "configurations:Debug"
        defines { "TRACE", "DEBUG", "CSHARP" }
        symbols "On"

    filter "configurations:Release"
        defines { "TRACE", "RELEASE", "CSHARP" }
        symbols "On"
        optimize "On"

    filter "configurations:Final"
        defines { "TRACE", "FINAL", "CSHARP" }
        optimize "On"

    filter "files:**.hlsl"
        buildaction "Embed"
