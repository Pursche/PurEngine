CONTENT_NAME = "Content"

project(CONTENT_NAME)
    kind "Utility"
    location "build"
    filename (CONTENT_NAME .. _AMD_VS_SUFFIX)
    uuid "3F3B938C-ABC6-0051-B4D7-834520E25C56"
    targetdir "../bin/Data/"
    dependson { COOKER_NAME }

    files { "content/**.lua" }
    --disablewarnings { "3245" }

    --postbuildcommands { "..\\..\\bin\\Cooker.exe ..\\..\\bin\\Data\\ ..\\source\\"}

    -- Lua files will be ignored
    filter("files:**.lua")
        buildmessage " cooking %{file.relpath}"
        buildcommands { "..\\..\\bin\\Cooker.exe ..\\..\\bin\\Data\\ %{file.relpath}"}
        buildoutputs { "test.asd" }