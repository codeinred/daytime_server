#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

#include <string>

#include <async/connect.hpp>
#include <async/read_some.hpp>

#include <conduit/coroutine.hpp>

using boost::asio::ip::tcp;
using namespace conduit;
namespace asio = boost::asio;

coroutine read_and_print(tcp::socket socket) {
    std::array<char, 1024> buffer;
    std::string result;
    while (true) {
        auto [status, msg] = co_await async::read_some(socket, buffer);

        if (status == boost::asio::error::eof) {
            break;
        }
        if (status) {
            std::cerr << "Read error: " << status;
        }

        result += msg;
    }
    std::cout << result;
}

coroutine connect_and_read(asio::io_context& context, std::string host,
                           std::string service) {
    tcp::resolver resolver(context);
    tcp::socket socket(context);
    for (auto&& endpoint : resolver.resolve(host, service)) {
        auto& status = co_await async::connect(endpoint, socket);

        if (!status) {
            read_and_print(std::move(socket));
            co_return;
        }
    }
    std::cerr << "Could not resolve host\n";
}
int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: client <host>" << std::endl;
            return 1;
        }

        boost::asio::io_context context;

        auto run = [&]() { context.run(); };

        for (int i = 0; i < 1000; i++)
            connect_and_read(context, argv[1], "daytime");

        run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
