
getDir=function(str,sep)
    sep=sep or'/'
    return str:match("(.*"..sep..")")
end

function CompileCapNProto(workDir, matchPattern, capnpPath, compilerName)
    -- Temporarily set working dir
    local originalWorkDir = os.getcwd()
    os.chdir(workDir)

    -- Remove old CapNProto generated files, if they have the correct name (C++ does, C# doesn't)
    --local oldFiles = os.matchfiles("**.capnp.*")
    --for key, value in pairs(oldFiles) do
    --    os.remove(value)
    --end

    -- Figure out what files and dirs we want to run on
    local capnprotoFiles = os.matchfiles(matchPattern)
    local capnpDir = getDir(capnpPath, '\\')
    local compiler = capnpDir .. "capnpc-" .. compilerName .. ".exe"
    local ignorePath = getDir(matchPattern)

    local createdFiles = {}

    for key,value in pairs(capnprotoFiles) do
        local fileName = GetFileName(value)

        local valueWithoutIgnoredPart = value:gsub(ignorePath, "")
        local outputDir = getDir(valueWithoutIgnoredPart)

        local outputFilename = fileName .. GetExtension(compilerName)
        local outputPath = outputDir .. outputFilename
        local realOutputPath = os.realpath(outputPath)

        local shouldCompile = true
        -- If produced file exists
        if (os.isfile(realOutputPath)) then
            shouldCompile = false
            local prototypeStat = os.stat(value)
            local producedStat = os.stat(realOutputPath)
            
            -- Check if prototype hasn't been modified since the produced file was modified, continue loop
            if (prototypeStat.mtime >= producedStat.mtime) then
                shouldCompile = true
            end
        end

        if (shouldCompile) then
            -- Create any dirs needed
            os.mkdir(outputDir)

            -- Print a nice message detailing what we're trying to compile into what
            print(valueWithoutIgnoredPart .. " -> " .. outputPath)

            -- Compile the prototype
            local command = "\"" .. capnpPath .. "\" compile -o" .. os.realpath(compiler) .. " --src-prefix=" .. ignorePath .. " " .. value .. ""
            os.execute(command)

            table.insert(createdFiles, realOutputPath)
        end
    end
    -- The C++ generator creates ".capnp.c++" files instead of ".capnp.cpp" files for some incredibly stupid reason, let's fix that!
    if compilerName == "c++" then
        local cppFiles = os.matchfiles("**.c++")
        for key,value in pairs(cppFiles) do
            local newFileName = value:sub(0, value:len()-4) .. ".cpp"

            if (os.isfile(newFileName)) then
                os.remove(newFileName)
            end

            local result, error = os.rename(value, newFileName)
            if (result == nil) then
                print(newFileName .. " Rename error: " .. error);
            end
        end
    end

    -- The C# generator creates ".cs" files instead of ".capnp.cs" files following the naming scheme of the cpp generator, let's fix that!
    if compilerName == "csharp" then
        for key,value in pairs(createdFiles) do
            local oldFileName = value:sub(0, value:len()-9) .. ".cs"

            -- Remove old renamed file if necessary
            if (os.isfile(value)) then
                os.remove(value)
            end

            -- Rename file
            print("Renaming: " .. oldFileName .. " to " .. value)
            local result, error = os.rename(oldFileName, value)
            if (result == nil) then
                print(value .. " Rename error: " .. error);
            end
        end
    end

    -- Restore our workdir
    os.chdir(originalWorkDir)
end

function GetExtension(compilerName)
    if (compilerName == "csharp") then
        return ".cs"
    elseif (compilerName == "c++") then
        return ".h"
    else
        return ".unknown"
    end
end

function GetFileName(url)
    return url:match("^.+/(.+)$")
end