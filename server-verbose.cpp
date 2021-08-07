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

int get_thread_num() {
    static int thread_num = 0;
    static thread_local int thread_number = thread_num++;
    return thread_number;
}

static std::atomic_int messages = 0;
coroutine do_write(tcp::socket socket, std::string message) {
    int message_num = ++messages;
    std::cout << ("Beginning write of message no. " +
                  std::to_string(message_num) + " on thread " +
                  std::to_string(get_thread_num()) + '\n');
    auto&& [status, write_size] = co_await async::write(socket, message);

    if (status) {
        std::cerr << ("Write error: " + status.message() + '\n');
        co_return;
    }
    std::cout << ("Completing write of message no. " +
                  std::to_string(message_num) + " on thread " +
                  std::to_string(get_thread_num()) + '\n');
}

coroutine start_server(asio::io_context& context, tcp::acceptor& acceptor) {
    while (messages < 1000) {
        std::cout << ("Waiting for incoming connection on " +
                      std::to_string(get_thread_num()) + '\n');

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
        std::vector<std::thread> threads(num_threads);

        for (std::thread& t : threads) {
            t = std::thread([&] {
                start_server(context, acceptor);
                context.run();
            });
        }

        for (auto& t : threads) {
            t.join();
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
