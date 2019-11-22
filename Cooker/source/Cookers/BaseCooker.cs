using System;
using System.Collections.Generic;
using System.Dynamic;
using System.Linq;
using Microsoft.Scripting.Hosting;
using Neo.IronLua;

namespace Cooker.Cookers
{
    static class Env
    {

        public static bool TryGet<T>(dynamic environment, string name, out T result)
        {
            result = default(T);
            object property = environment[name];

            if (property != null)
            {
                try
                {
                    result = (T)property;
                    return true;
                }
                catch(Exception)
                {
                    
                }
            }

            return false;
        }

        public static bool TryGetArray<T>(dynamic environment, string name, out T[] result)
        {
            result = default;
            object property = environment[name];

            if (property == null || !(property is LuaTable))
                return false;

            LuaTable table = (LuaTable)property;
            int arrayListCount = table.ArrayList.Count;
            result = new T[arrayListCount];

            for(int i = 0; i < arrayListCount; i++)
            {
                try
                {
                    result[i] = (T)table.ArrayList[i];
                }
                catch(Exception)
                {
                    result[i] = default;
                }
            }

            return true;
        }

        public static T TryGetDefault<T>(dynamic environment, string name, T def)
        {
            object property = environment[name];

            if (property != null)
            {
                try
                {
                    T result = (T)property;
                    return result;
                }
                catch (Exception)
                {

                }
            }

            return def;
        }

        public static T[] TryGetArrayDefault<T>(dynamic environment, string name, T[] def)
        {
            object property = environment[name];

            if (property == null || !(property is LuaTable))
                return def;
            
            try
            {
                LuaTable table = (LuaTable)property;
                int arrayListCount = table.ArrayList.Count;
                T[] result = new T[arrayListCount];

                for (int i = 0; i < arrayListCount; i++)
                {
                    try
                    {
                        result[i] = (T)table.ArrayList[i];
                    }
                    catch (Exception)
                    {
                        result[i] = default;
                    }
                }

                return result;
            }
            catch (Exception)
            {

            }

            
            return def;
        }

        
    }

    abstract class BaseCooker
    {
        abstract public void Init();
        abstract public void RegisterFunctions(LuaGlobal environment);
        abstract public bool CanCook(string luaPath, LuaGlobal environment, out string error);
        abstract public bool Cook(string luaPath, LuaGlobal environment, string outputDirectory, string intermediateDirectory, out string producedFile, out string error);

        protected string GetAssetPath(string luaPath)
        {
            int luaExtIndex = luaPath.LastIndexOf(".lua");
            return luaPath.Substring(0, luaExtIndex);
        }
    }
}