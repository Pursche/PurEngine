
getDir=function(str,sep)
    sep=sep or'/'
    return str:match("(.*"..sep..")")
end

function CompileCapNProto(workDir, matchPattern, capnpPath, compilerName)
    -- Temporarily set working dir
    local originalWorkDir = os.getcwd()
    os.chdir(workDir)

    -- Remove old CapNProto generated files, if they have the correct name (C++ does, C# doesn't)
    local oldFiles = os.matchfiles("**.capnp.*")
    for key, value in pairs(oldFiles) do
        os.remove(value)
    end

    -- Figure out what files and dirs we want to run on
    local capnprotoFiles = os.matchfiles(matchPattern)
    local capnpDir = getDir(capnpPath, '\\')
    local compiler = capnpDir .. "capnpc-" .. compilerName .. ".exe"
    local ignorePath = getDir(matchPattern)

    for key,value in pairs(capnprotoFiles) do
        -- Create any dirs needed
        local valueWithoutIgnoredPart = value:gsub(ignorePath, "")
        local outputDir = getDir(valueWithoutIgnoredPart)
        os.mkdir(outputDir)

        -- Print a nice message detailing what we're trying to compile into what
        local fileName = GetFileName(value)
        local outputFilename = fileName .. GetExtension(compilerName)
        print(valueWithoutIgnoredPart .. " -> " .. outputDir .. outputFilename)

        -- Compile the prototype
        local command = "\"" .. capnpPath .. "\" compile -o" .. os.realpath(compiler) .. " --src-prefix=" .. ignorePath .. " " .. value .. ""
        os.execute(command)
    end

    -- The C++ generated creates ".c++" files instead of ".cpp" files for some incredibly stupid reason, let's fix that!
    if compilerName == "c++" then
        local cppFiles = os.matchfiles("**.c++")
        for key,value in pairs(cppFiles) do
            local newFileName = value:sub(0, value:len()-4) .. ".cpp"
            os.rename(value, newFileName)
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