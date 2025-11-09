#pragma once

#include <functional>
#include <utility>

namespace euler {

template <typename F>
    requires std::is_invocable_r_v<void, F>
class Defer {
public:
    Defer(F&& fn) : fn{std::move(fn)} {}
    ~Defer() {
        if (!should_run) std::invoke(fn);
    }

    void defuse() { should_run = true; }

private:
    bool should_run = false;
    F fn;
};

}  // namespace hornet