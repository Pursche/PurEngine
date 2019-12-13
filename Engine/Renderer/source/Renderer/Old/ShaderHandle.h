#pragma once
#include <Utils/StrongTypedef.h>
#include <limits>

template <class StrongTypedef>
struct ShaderComparison
{
    friend bool operator==(StrongTypedef& lhs, const StrongTypedef& rhs)
    {
        using type = type_safe::underlying_type<StrongTypedef>;
        return static_cast<type&>(lhs) == static_cast<const type&>(rhs);
    }

    friend bool operator==(const StrongTypedef& lhs, const StrongTypedef& rhs)
    {
        using type = type_safe::underlying_type<StrongTypedef>;
        return static_cast<type&>(lhs) == static_cast<const type&>(rhs);
    }

    friend bool operator!=(StrongTypedef& lhs, const StrongTypedef& rhs)
    {
        using type = type_safe::underlying_type<StrongTypedef>;
        return static_cast<type&>(lhs) != static_cast<const type&>(rhs);
    }

    friend bool operator!=(const StrongTypedef& lhs, const StrongTypedef& rhs)
    {
        using type = type_safe::underlying_type<StrongTypedef>;
        return static_cast<type&>(lhs) != static_cast<const type&>(rhs);
    }
};

// If you want to change the underlying integer type used for these handles, change the type under "HERE"
//                                                                HERE
struct __ShaderHandle : type_safe::strong_typedef<__ShaderHandle, u16>, ShaderComparison<__ShaderHandle>
{
    using strong_typedef::strong_typedef;
    using type = type_safe::underlying_type<__ShaderHandle>;

    static size_t MaxValue()
    {
        return std::numeric_limits<type>::max();
    }

    static type MaxValueTyped()
    {
        return (type)std::numeric_limits<type>::max();
    }
};

struct ShaderHandleVertex :type_safe::strong_typedef<ShaderHandleVertex, __ShaderHandle>, ShaderComparison<ShaderHandleVertex>
{
    using strong_typedef::strong_typedef;

    static ShaderHandleVertex Invalid()
    {
        return ShaderHandleVertex(static_cast<ShaderHandleVertex>(__ShaderHandle(__ShaderHandle::MaxValueTyped())));
    }
};

struct ShaderHandlePixel :type_safe::strong_typedef<ShaderHandlePixel, __ShaderHandle>, ShaderComparison<ShaderHandlePixel>
{
    using strong_typedef::strong_typedef;

    static ShaderHandlePixel Invalid()
    {
        return ShaderHandlePixel(static_cast<ShaderHandlePixel>(__ShaderHandle(__ShaderHandle::MaxValueTyped())));
    }
};

struct ShaderHandleCompute :type_safe::strong_typedef<ShaderHandleCompute, __ShaderHandle>, ShaderComparison<ShaderHandleCompute>
{
    using strong_typedef::strong_typedef;

    static ShaderHandleCompute Invalid()
    {
        return ShaderHandleCompute(static_cast<ShaderHandleCompute>(__ShaderHandle(__ShaderHandle::MaxValueTyped())));
    }
};