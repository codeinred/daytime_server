#include <async/accept.hpp>
#include <async/write.hpp>
#include <conduit/coroutine.hpp>

#include <iostream>

using boost::asio::ip::tcp;
using boost::system::error_code;
namespace asio = boost::asio;
namespace async = conduit::async;
using conduit::coroutine;

std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

coroutine start_write(tcp::socket socket, std::string message) {
    co_await async::write(socket, message);
}

coroutine start_server(asio::io_context& context, tcp::acceptor& acceptor) {
    while (true) {
        auto socket = tcp::socket(context);
        error_code const& status = co_await async::accept(acceptor, socket);

        if (!status)
            start_write(std::move(socket), make_daytime_string());
        else
            std::cerr << "Server error: " << status.message() << '\n';
    }
}

int main(int argc, char** argv) {
    try {
        boost::asio::io_context context;
        auto endpoint = tcp::endpoint(tcp::v4(), 13);
        auto acceptor = tcp::acceptor(context, endpoint);

        start_server(context, acceptor);
        context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
