#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

#include <string>

#include <async/connect.hpp>
#include <async/read.hpp>
#include <async/resolve.hpp>

#include <conduit/coroutine.hpp>
#include <conduit/future.hpp>

using boost::asio::ip::tcp;
using namespace conduit;
namespace asio = boost::asio;

coroutine get_daytime(asio::io_context& context, std::string host,
                           std::string service) {
    tcp::resolver resolver(context);
    async::resolve_result r = co_await async::resolve(resolver, host, service);

    tcp::socket socket(context);
    auto&& [status, endpoint] = co_await async::connect(socket, r.endpoints);

    std::string message = co_await async::read(socket);

    std::cout << message;
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: client <host>" << std::endl;
            return 1;
        }

        boost::asio::io_context context;

        get_daytime(context, argv[1], "daytime");

        context.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
