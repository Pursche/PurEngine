#pragma once
#include <Core.h>
#include <Utils/StrongTypedef.h>
#include <limits>

template <class StrongTypedef>
struct PipelineComparison
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

// If you want to change the underlying integer type used for this handle, change the type under "HERE"
//                                                                HERE
struct PipelineHandle : type_safe::strong_typedef<PipelineHandle, u16>, PipelineComparison<PipelineHandle>
{
    using strong_typedef::strong_typedef;
    using type = type_safe::underlying_type<PipelineHandle>;

    static size_t MaxValue()
    {
        return std::numeric_limits<type>::max();
    }

    static type MaxValueTyped()
    {
        return (type)std::numeric_limits<type>::max();
    }

    static PipelineHandle Invalid()
    {
        return PipelineHandle(PipelineHandle(PipelineHandle::MaxValueTyped()));
    }
};