#pragma once
#include <concepts>
#include <string_view>
#include <type_traits>

#include <boost/system/error_code.hpp>

namespace boost::system {
class error_code;
}
namespace conduit::async {
using boost::system::error_code;
struct status_result {
    error_code const& status;
};
struct write_result {
    error_code const& status;
    // Bytes written
    size_t count;
};
struct read_result {
    error_code const& status;
    std::string_view message;
};
template <class F, class T>
concept value_or_error_visitor = requires(F func, T t, error_code const& c) {
    { func(t) }
    ->std::common_with<decltype(func(c))>;
};
template <class T>
struct wrap {
    T value;

    constexpr T& get() & noexcept { return value; }
    constexpr T&& get() && noexcept { return std::move(value); }
    constexpr T const& get() const& noexcept { return value; }
    using reference = T&;
    using const_reference = T const&;
    using move_reference = T&&;
};
template <class T>
struct wrap<T&> {
    T* pointer = nullptr;
    constexpr wrap(T& value) noexcept : pointer(&value) {}
    constexpr T& get() const noexcept { return *pointer; }
    using reference = T&;
    using const_reference = T&;
    using move_reference = T&;
};
template <class T>
class value_or_error {
    constexpr const static error_code ok = error_code();
    union {
        wrap<T> w;
        error_code const* error;
    };
    bool has_value_ = false;
   public:
    using reference = typename wrap<T>::reference;
    using const_reference = typename wrap<T>::const_reference;
    using move_reference = typename wrap<T>::move_reference;
    value_or_error() = delete;
    value_or_error(T const& value) : w(value), has_value_(true) {}
    value_or_error(T&& value) : w(std::move(value)), has_value_(true) {}
    value_or_error(error_code const& e) : error(&e), has_value_(false) {}
    value_or_error(error_code const& e, T const& value) : error(&e), has_value_(false) {
        if(!e) {
            new(&w) wrap(value);
        }
    }
    
    auto& operator=(value_or_error const& other) {
        if(has_value_) {
            if(other.has_value) {
                w = other.w;
            } else {
                w.~wrap();
                error = other.error;
                has_value_ = false;
            }
        } else {
            if(other.has_value) {
                new(&w) wrap(other.w);
                has_value_ = true;
            } else {
                error = other.error;
            }
        }
        return *this;
    }
    auto& operator=(value_or_error&& other) {
        if(has_value_) {
            if(other.has_value) {
                w = std::move(other.w);
            } else {
                w.~wrap();
                error = other.error;
                has_value_ = false;
            }
        } else {
            if(other.has_value) {
                new(&w) wrap(std::move(other.w));
                has_value_ = true;
            } else {
                error = other.error;
            }
        }
        return *this;
    }
    decltype(auto) visit(value_or_error_visitor<T> auto func) {
        return has_value_ ? func(w.get()) : func(*error);
    }
    error_code const& get_error() const {
        if(has_value_) {
            return ok;
        }
        return *error;
    }
    bool has_value() const noexcept {
        return has_value_;
    }
    operator bool() const noexcept {
        return has_value_;
    }
    decltype(auto) value() & {
        if(!has_value_) {
            throw *error;
        }
        return w.get();
    }
    decltype(auto) value() const& {
        if(!has_value_) {
            throw *error;
        }
        return w.get();
    }
    decltype(auto) value() && {
        if(!has_value_) {
            throw *error;
        }
        return std::move(w).get(); 
    }
    ~value_or_error() {
        if(has_value_) {
            w.~wrap();
        }
    }
};
} // namespace async
