#include <async/accept.hpp>
#include <async/write.hpp>
#include <conduit/coroutine.hpp>

#include <atomic>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;
namespace asio = boost::asio;
using namespace conduit;

std::string make_daytime_string() {
    using namespace std; // For time_t, time and ctime;
    time_t now = time(0);
    return ctime(&now);
}

coroutine do_write(tcp::socket socket, std::string message) {
    auto&& [status, write_size] = co_await async::write(socket, message);

    if (status) {
        std::cerr << ("Write error: " + status.message() + '\n');
        co_return;
    }
}

coroutine start_server(asio::io_context& context, tcp::acceptor& acceptor) {
    while (true) {
        // Accept an incoming connection
        auto socket = tcp::socket(context);
        auto& status = co_await async::accept(acceptor, socket);

        if (status) {
            std::cerr << ("Server error: " + status.message() + '\n');
            continue;
        }

        // Transfer ownership of the socket to the write command
        do_write(std::move(socket), make_daytime_string());
    }
    context.stop();
}

int main(int argc, char** argv) {
    try {
        boost::asio::io_context context;
        auto endpoint = tcp::endpoint(tcp::v4(), 13);
        auto acceptor = tcp::acceptor(context, endpoint);

        int num_threads = 8;
        std::vector<std::jthread> threads(num_threads);

        for (auto& t : threads) {
            t = std::jthread([&] {
                start_server(context, acceptor);
                context.run();
            });
        }


        std::cout << "Type 'halt' to halt the server or press Crtl+D." << std::endl;
        std::string cin_buffer(1024, '\0');
        // Wait until EOF from std::cin
        while(std::cin) {
            std::cout << "> " << std::flush;
            std::getline(std::cin, cin_buffer);
            if(cin_buffer == "halt")
                break;
        }
        std::cerr << "Stopping" << '\n';
        // Cancel all coroutines
        context.stop();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "A daytime server runs on port 13, so it needs to be run with sudo. Try:\n";
        std::cerr << "\n  $ sudo " << argv[0] << '\n';
    }

    return 0;
}
