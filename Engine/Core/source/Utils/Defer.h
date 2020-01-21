#pragma once

namespace Utils
{
    template <typename F>
    struct PurDefer 
    {
        F f;
        PurDefer(F f) : f(f) {}
        ~PurDefer() { f(); }
    };

    template <typename F>
    PurDefer<F> DeferFunc(F f)
    {
        return PurDefer<F>(f);
    }
}

#define _DEFER1(x, y) x##y
#define _DEFER2(x, y) _DEFER1(x, y)
#define _DEFER3(x)    _DEFER2(x, __COUNTER__)
#define PurDefer(code)   auto _DEFER3(_defer_) =     Utils::DeferFunc([&](){code;})