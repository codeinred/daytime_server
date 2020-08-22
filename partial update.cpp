//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <ctime>
#include <iostream>
#include <minimal_promise.hpp>
#include <string>

using boost::asio::ip::tcp;
using boost::system::error_code;

namespace asio = boost::asio;

std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

struct accept_awaitable {
    tcp::acceptor& acceptor;
    tcp::socket& socket;
    boost::system::error_code const* error_ptr = nullptr;
    constexpr bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> handle) {
        acceptor.async_accept(socket, [handle, this](boost::system::error_code const& error) {
            error_ptr = &error;
            handle.resume();
        });
    }
    boost::system::error_code await_resume() { return *error_ptr; }
};

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
   public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_context& io_context) {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket& socket() { return socket_; }

    void start() {
        message_ = make_daytime_string();

        boost::asio::async_write(socket_, boost::asio::buffer(message_),
                                 boost::bind(&tcp_connection::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }

   private:
    tcp_connection(boost::asio::io_context& io_context) : socket_(io_context) {}

    void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/) {}

    tcp::socket socket_;
    std::string message_;
};

auto run_tcp_server(boost::asio::io_context& io_context) -> minimal_coro {
    auto acceptor = tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), 13));

    while (true) {
        auto new_connection = tcp_connection::create(io_context);
        auto error = co_await accept_awaitable{acceptor, new_connection->socket()};

        if (!error) {
            new_connection->start();
        }
    }
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp_server server(io_context);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
