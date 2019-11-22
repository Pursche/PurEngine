#pragma once
#include "../Core.h"
#include <string>

namespace StringUtils
{
    // FNV-1a 32bit hashing algorithm.
    constexpr u32 fnv1a_32(char const* s, std::size_t count)
    {
        return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
    }

    std::wstring StringToWString(const std::string& as);
    
} // namespace StringUtils

constexpr u32 operator"" _h(char const* s, std::size_t count)
{
    return StringUtils::fnv1a_32(s, count);
}