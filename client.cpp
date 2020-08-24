#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

#include <string>

#include <conduit/io/connect.hpp>
#include <conduit/io/read.hpp>
#include <conduit/io/resolve.hpp>

#include <conduit/coroutine.hpp>
#include <conduit/future.hpp>

using boost::asio::ip::tcp;
using namespace conduit;
namespace asio = boost::asio;

coroutine get_daytime(asio::io_context& context, std::string host,
                      std::string service) {
    tcp::resolver resolver(context);
    io::resolve_result r = co_await io::resolve(resolver, host, service);

    tcp::socket socket(context);
    co_await io::connect(socket, r.endpoints);

    auto get_msg = io::read(socket);
    std::string message = co_await get_msg;

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
