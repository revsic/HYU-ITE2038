#ifndef UTILS_HPP
#define UTILS_HPP

#include <atomic>
#include <functional>

/// Utilities.
namespace utils {
/// Structure for parameter placeholder.
struct token_t {};
/// Token object.
constexpr token_t token;

/// `defer` keywords from golang.
template <typename F>
struct Defer {
    /// callback on destructor.
    F callback;

    /// Constructor.
    /// \tparam typename Fp, callback type.
    /// \param callback Fp&&, callback.
    template <typename Fp>
    Defer(Fp&& callback) : callback(std::forward<Fp>(callback)) {
        // Do Nothing
    }
    /// Destructor.
    ~Defer() {
        callback();
    }

    /// Deleted copy destructor.
    Defer(Defer const&) = delete;
    /// Move constructor.
    Defer(Defer&& other) noexcept : callback(std::move(other.callback)) {
        // Do Nothing
    }

    /// Deleted copy assignment.
    Defer& operator=(Defer const&) = delete;
    /// Deleted move assignment.
    Defer& operator=(Defer&&) = delete;
};

/// `defer` keywords in golang.
/// \tparam typename F, callback type.
/// \param functor F&&, callback.
/// \return Defer<F>, defer object.
template <typename F>
auto defer(F&& functor) {
    return Defer<F>(std::forward<F>(functor));
}

}

#endif