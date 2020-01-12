--shaderFile = "test.vs.hlsl" -- Lets the user override the asset path so the .lua file doesn't have to have the same name as the shader
shaderType = "vs" -- Maybe this should set -T <profile> instead?
entryPoint = "main" -- -E <value>, entry point name
optimization = "Od" -- Od, O0, O1, O2, O3 flags, O3 default
profile = "vs_6_4" -- -T <profile> <profile>: ps_6_0, ps_6_1, ps_6_2, ps_6_3, ps_6_4, ps_6_5, vs_6_0, vs_6_1, vs_6_2, vs_6_3, vs_6_4, vs_6_5, gs_6_1, gs_6_2, gs_6_3, gs_6_4, gs_6_5, ds_6_0, ds_6_1, ds_6_2, ds_6_3, ds_6_4, ds_6_5, hs_6_0, hs_6_1, hs_6_2, hs_6_3, hs_6_4, hs_6_5, lib_6_3, lib_6_4, lib_6_5, ms_6_5, as_6_5
debug = true -- -Zi if true
--outputFile = "asd" -- -Fo, output file name
defines = { "foo", "bar" } -- -D <value>, defines

--setMutation("TEST_VAL", 1.0)
--setMutation("TEST_BLANK", "")

function OnCompile(settings)
	return compileShader(settings);
end

-- TODO: Run dxc.exe -help to find more interesting things to support here
-- dxc.exe [options] <inputs>