#pragma once
#include <async/responses.hpp>
#include <conduit/async/callback.hpp>
#include <conduit/mixin/resumable.hpp>

#include <boost/asio/basic_socket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>

#include <coroutine>
#include <string>

namespace conduit::async {
using boost::asio::ip::tcp;
namespace asio = boost::asio;

class write : public mixin::Resumable<write> {
    std::string_view message;
    tcp::socket* socket = nullptr;
    error_code const* status;

    auto get_handler(std::coroutine_handle<> h) {
        return [this, caller = async::callback{h}](error_code const& e,
                                                   size_t s) mutable {
            status = &e;
            message = message.substr(0, s);
            caller.resume();
        };
    }
    void on_suspend(std::coroutine_handle<> h) {
        asio::async_write(*socket, asio::buffer(message),
                          get_handler(h));
    }
    friend class mixin::Resumable<write>;

   public:
    write() = default;
    write(write const&) = default;
    write(tcp::socket& socket, std::string_view data)
      : socket(&socket), message(data) {}

    write_result await_resume() { return {*status, message.size()}; }
}; // namespace conduit::async
} // namespace conduit::async
