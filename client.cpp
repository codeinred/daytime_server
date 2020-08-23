#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

#include <string>

#include <async/connect.hpp>
#include <async/read_some.hpp>
#include <async/resolve.hpp>

#include <conduit/coroutine.hpp>
#include <conduit/future.hpp>

using boost::asio::ip::tcp;
using namespace conduit;
namespace asio = boost::asio;

future<std::string> read(tcp::socket& socket) {
    std::array<char, 1024> buffer;
    std::string result;
    while (auto response = co_await async::read_some(socket, buffer)) {
        result += response.message;

        if (response.status) {
            std::cerr << response.status.message() << '\n';
            break;
        }
    }
    co_return result;
}

coroutine connect_and_read(asio::io_context& context, std::string host,
                           std::string service) {
    tcp::resolver resolver(context);
    async::resolve_result r = co_await async::resolve(resolver, host, service);

    tcp::socket socket(context);
    auto&& [status, endpoint] = co_await async::connect(socket, r.endpoints);

    std::string message = co_await read(socket);

    std::cout << message << '\n';
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: client <host>" << std::endl;
            return 1;
        }

        boost::asio::io_context context;

        connect_and_read(context, argv[1], "daytime");

        context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
