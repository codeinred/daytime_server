#pragma once

#include <async/responses.hpp>

#include <boost/asio/basic_socket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <conduit/continuation.hpp>
#include <conduit/util/stdlib_coroutine.hpp>

namespace conduit::async {
using boost::asio::ip::tcp;

class connect {
    tcp::endpoint const* endpoint = nullptr;
    tcp::socket* socket_ptr = nullptr;
    error_code const* status = nullptr;

    auto get_handler(std::coroutine_handle<> h) {
        return [this, caller = async::callback{h}](
                   error_code const& response) mutable {
            status = &response;
            caller.resume();
        };
    }
   public:
    connect() = default;
    connect(connect const&) = default;
    connect(tcp::endpoint const& endpoint, tcp::socket& socket)
        : endpoint(&endpoint), socket_ptr(&socket) {}

    constexpr bool await_ready() const noexcept {
        return false;
    }
    void await_suspend(std::coroutine_handle<> h) {
        socket_ptr->async_connect(*endpoint, get_handler(h));
    }
    error_code const& await_resume() { return *status; }
};
} // namespace async
