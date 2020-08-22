#pragma once

#include <async/responses.hpp>

#include <boost/asio/ip/tcp.hpp>

#include <coroutine>


namespace async {
using boost::asio::ip::tcp;
struct ready_t {};
struct suspend_t {};

class resolve : tcp::resolver {
    using super = tcp::resolver;
    public:
    using tcp::resolver::resolver;

    constexpr bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> caller) {
    }

};
}
