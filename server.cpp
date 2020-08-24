#include <conduit/net/accept.hpp>
#include <conduit/net/write.hpp>
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

coroutine start_server(asio::io_context& context, tcp::acceptor& acceptor) {
    for (int i = 0; i < 10; i++) {
        auto socket = tcp::socket(context);
        auto status = co_await async::accept(acceptor, socket);

        if (status.bad())
            std::cerr << "Server error: " << status.message() << '\n';

        async::start_write(std::move(socket), make_daytime_string());
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
