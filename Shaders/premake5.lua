SHADERS_NAME = "Shaders"

project(SHADERS_NAME)
    kind "Utility"
    location "build"
    filename (SHADERS_NAME .. _AMD_VS_SUFFIX)
    uuid "3F3B938C-ABC6-0051-B4D7-834520E25C53"
    targetdir "../bin/Data/shaders"
    dependson { COOKER_NAME }

    files { "source/**.hlsl", "source/**.lua" }
    disablewarnings { "3245" }

    --postbuildcommands { "..\\..\\bin\\Cooker.exe ..\\..\\bin\\Data\\ ..\\source\\"}

    -- Lua files will be ignored
    filter("files:**.lua")
        buildmessage " cooking %{file.relpath}"
        buildcommands { "..\\..\\bin\\Cooker.exe ..\\..\\bin\\Data\\ %{file.relpath}"}
        buildoutputs { "test.asd" }

    -- HLSL files that don't end with 'Extensions' will be ignored as they will be
    -- used as includes
    filter("files:**.hlsl")
        flags("ExcludeFromBuild")

    filter("files:**_ps.hlsl")
        removeflags("ExcludeFromBuild")

    filter("files:**_vs.hlsl")
        removeflags("ExcludeFromBuild")

    filter("files:**_cs.hlsl")
        removeflags("ExcludeFromBuild")