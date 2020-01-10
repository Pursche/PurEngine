using Neo.IronLua;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;

namespace Cooker
{
    class Program
    {
        static void LuaPrint(object[] texts)
        {
            Console.Write("[LUA]: ");
            foreach (object o in texts)
                Console.Write(o);
            Console.WriteLine();
        }

        static void Main(string[] args)
        {
            Console.ForegroundColor = ConsoleColor.White;

            if (args.Length < 2)
            {
                Console.Error.WriteLine("[ERROR] Cooker expected at least 2 argument while only " + args.Length + " was supplied!");
                for(int i = 0; i < args.Length; i++)
                {
                    Console.WriteLine(args[i]);
                }
                Environment.Exit(1);
                return;
            }

            string outputDirectory = Path.GetFullPath(args[0]);
            Directory.CreateDirectory(outputDirectory);

            // Create intermediate directory
            string intermediateDirectory = Path.GetFullPath(Path.Combine(outputDirectory, @"..\..\build\intermediate"));
            Directory.CreateDirectory(intermediateDirectory);

            // Create all Cookers
            IEnumerable<Cookers.BaseCooker> cookers = ReflectiveEnumerator.GetEnumerableOfType<Cookers.BaseCooker>();
            foreach (Cookers.BaseCooker cooker in cookers)
            {
                cooker.Init();
            }

            Lua luaEngine = new Lua();

            Stopwatch totalTime = Stopwatch.StartNew();

            List<string> luaFiles = new List<string>();

            for (int i = 1; i < args.Length; i++)
            {
                string dataArg = args[i];
                FileAttributes attributes = File.GetAttributes(dataArg);

                if (attributes.HasFlag(FileAttributes.Directory))
                {
                    // Path supplied is a directory, we need to scan it for lua files
                    string[] foundLuaFiles = Directory.GetFiles(dataArg, "*.lua", SearchOption.AllDirectories);
                    luaFiles.AddRange(foundLuaFiles);
                }
                else
                {
                    // Path supplied is a file, verify that it is a .lua file and add it
                    Debug.Assert(dataArg.EndsWith(".lua"));
                    luaFiles.Add(dataArg);
                }
            }

            int errorCount = 0;
            foreach (string luaFile in luaFiles)
            {
                string output = luaFile;

                Stopwatch scriptTime = Stopwatch.StartNew();

                LuaGlobal environment = luaEngine.CreateEnvironment<LuaGlobal>();
                dynamic dynamicEnvironment = environment;
                dynamicEnvironment.print = new Action<object[]>(LuaPrint);

                foreach (Cookers.BaseCooker cooker in cookers)
                {
                    cooker.RegisterFunctions(environment);
                }

                LuaChunk chunk = null;
                try
                {
                    chunk = luaEngine.CompileChunk(luaFile, new LuaCompileOptions() { DebugEngine = LuaStackTraceDebugger.Default });
                }
                catch(Neo.IronLua.LuaParseException e)
                {
                    Debug.Fail("[LUA ERROR] " + e.Message + "\n" + e.FileName + " (line " + e.Line + ")");
                }

                try
                {
                    LuaResult result = environment.DoChunk(chunk);
                }
                catch (Exception e)
                {
                    Console.WriteLine("Expception: {0}", e.Message);
                    var d = LuaExceptionData.GetData(e); // get stack trace
                    Console.WriteLine("StackTrace: {0}", d.FormatStackTrace(0, false));
                }

                List<string> errors = new List<string>();

                bool availableCooker = false;
                bool wasCooked = false;
                string producedFile = "";
                foreach (Cookers.BaseCooker cooker in cookers)
                {
                    string error;
                    if (cooker.CanCook(luaFile, environment, out error))
                    {
                        errors.Clear();
                        availableCooker = true;


                        if (cooker.Cook(luaFile, environment, outputDirectory, intermediateDirectory, out producedFile, out error))
                        {
                            wasCooked = true;
                        }
                        else if (error != "")
                        {
                            errors.Add(" (" + cooker.GetType().GetTypeInfo().Name + ") " + error);
                        }
                    }
                    else if (error != "")
                    {
                        errors.Add(" (" + cooker.GetType().GetTypeInfo().Name + ") " + error);
                    }
                }

                int pCode = 0;
                if (!availableCooker)
                {
                    output += "(0): error P1: file error: Could not find a suitable cooker";
                    pCode = 1;
                }
                else if (!wasCooked)
                {
                    output += "(0): error P2: file error: Could not successfully be cooked";
                    pCode = 2;
                }
                else
                {
                    //output += " Cooked successfully";
                    output += " --> " + producedFile;
                }

                output += " (" + scriptTime.Elapsed.ToString() + ")";

                if (availableCooker && wasCooked)
                {
                    Console.ForegroundColor = ConsoleColor.Green;
                    //Console.Write("[SUCCESS]: ");
                }
                else
                {
                    Console.ForegroundColor = ConsoleColor.Red;
                    //Console.Write("[ERROR]: ");
                    errorCount++;
                }
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine(output);

                if (!availableCooker || !wasCooked)
                {
                    foreach (string error in errors)
                    {
                        Console.WriteLine("(0): warning P" + pCode + " : " + error);
                    }
                }
            }

            if (luaFiles.Count > 1)
                Console.WriteLine("Cooking done in " + totalTime.Elapsed.ToString());

            if (errorCount > 0)
                Environment.Exit(1);
        }
    }
}