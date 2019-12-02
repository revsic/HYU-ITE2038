#ifndef UTILS_HPP
#define UTILS_HPP

#include <functional>

namespace utils {

struct Defer {
    std::function<void()> callback;

    template <typename F>
    Defer(F&& callback) : callback(std::forward<F>(callback)) {
        // Do Nothing
    }

    ~Defer() {
        callback();
    }

    Defer(Defer const&) = delete;

    Defer(Defer&&) = delete;

    Defer& operator=(Defer const&) = delete;

    Defer& operator=(Defer&&) = delete;
};

}

#endif