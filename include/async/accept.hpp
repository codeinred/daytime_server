#pragma once
#include <conduit/async/callback.hpp>
#include <conduit/mixin/resumable.hpp>

#include <async/responses.hpp>

#include <boost/asio/basic_socket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

namespace conduit::async {
using boost::asio::ip::tcp;

class accept : public mixin::Resumable<accept> {
    tcp::acceptor* acceptor = nullptr;
    tcp::socket* socket = nullptr;
    error_code const* status = nullptr;
    auto get_handler(std::coroutine_handle<> h) {
        return [this, caller = async::callback{h}](
                   error_code const& response) mutable {
            status = &response;
            caller.resume();
        };
    }
    void on_suspend(std::coroutine_handle<> h) {
        acceptor->async_accept(*socket, get_handler(h));
    }
    friend class mixin::Resumable<accept>;

   public:
    accept() = default;
    accept(accept const&) = default;
    accept(tcp::acceptor& acceptor, tcp::socket& socket)
      : acceptor(&acceptor), socket(&socket) {}

    error_code const& await_resume() noexcept { return *status; }
    // status_result await_resume() noexcept { return {*status}; }
};
} // namespace conduit::async
