#pragma once

#include <async/responses.hpp>

#include <boost/asio/basic_socket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>

#include <array>
#include <span>
#include <string>
#include <string_view>

#include <conduit/util/stdlib_coroutine.hpp>
#include <conduit/async/callback.hpp>

namespace conduit::async {
using boost::asio::ip::tcp;
namespace asio = boost::asio;

class read_some {
    template <size_t N>
    using char_buff = char[N];

    tcp::socket* socket = nullptr;
    error_code const* status = nullptr;

    std::span<char> buffer;


    auto get_handler(std::coroutine_handle<> h) {
        return [this, caller = async::callback{h}](error_code const& response,
                                                   size_t s) mutable {
            status = &response;
            buffer = buffer.subspan(0, s);
            caller.resume();
        };
    }


   public:
    read_some() = default;
    read_some(read_some const&) = default;
    read_some(tcp::socket& socket, std::span<char> buffer) noexcept
      : socket(&socket), buffer(buffer) {}

    constexpr bool await_ready() const noexcept {
        return false;
    }
    void await_suspend(std::coroutine_handle<> h) {
        socket->async_read_some(
            asio::mutable_buffer(buffer.data(), buffer.size()), get_handler(h));
    }
    read_result await_resume() {
        return {*status, std::string_view(buffer.data(), buffer.size())};
    }
};
} // namespace conduit::async
