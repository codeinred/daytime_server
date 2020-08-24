#include <conduit/coroutine.hpp>
#include <conduit/io/accept.hpp>
#include <conduit/io/write.hpp>

#include <boost/asio/signal_set.hpp>

#include <csignal>
#include <functional>
#include <iostream>
#include <string>

using boost::asio::ip::tcp;
using boost::system::error_code;
namespace asio = boost::asio;
namespace async = conduit::async;
using namespace conduit;

std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

coroutine start_server(asio::io_context& context, tcp::acceptor& acceptor) {
    while (true) {
        auto socket = tcp::socket(context);
        auto status = co_await io::accept(acceptor, socket);

        if (status.bad())
            std::cerr << "Server error: " << status.message() << '\n';

        io::start_write(std::move(socket), make_daytime_string());
    }
}

std::function<void(int)> on_sig_int = [](int) {};
void sigint(int s) { on_sig_int(s); }

int main(int argc, char** argv) {
    try {
        boost::asio::io_context context;
        auto endpoint = tcp::endpoint(tcp::v4(), 13);
        auto acceptor = tcp::acceptor(context, endpoint);

        start_server(context, acceptor);

        asio::signal_set signals(context, SIGINT);
        signals.async_wait([&](auto& ec, int sig) { context.stop(); });

        context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
