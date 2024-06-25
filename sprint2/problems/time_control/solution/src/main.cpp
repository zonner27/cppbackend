#include "sdk.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "json_loader/json_loader.h"
#include "request_handler/request_handler.h"
#include "files.h"
#include "logger/logger.h"
#include "app/players.h"
#include "app/application.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace http = boost::beast::http;

namespace {
// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

int main(int argc, const char* argv[]) {

    logger_init();

    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <game-folder>"sv << std::endl;
        return EXIT_FAILURE;
    }
    fs::path base_path = argv[0];
    base_path = base_path.parent_path() / "../../"s;
    fs::path json_path = argv[1];
    fs::path static_path = argv[2];

    if (!files_path::IsSubPath(static_path, base_path)) {
        std::cerr << "The folder "sv << argv[2] << " is not in the program directory"sv << std::endl;
        return EXIT_FAILURE;
    }
    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game;
        try {
            game = json_loader::LoadGame(json_path);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        //app::PlayerTokens playerTokens;
        //app::Players players;

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        app::Application application(game, ioc);


        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        // Подписываемся на сигналы и при их получении завершаем работу сервера
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
                json::value custom_data = json::object{{"code"s, 0}};
                BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "server exited"sv;
            }
        });

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        //http_handler::RequestHandler handler{game, static_path};
        //http_handler::LoggingRequestHandler logging_handler{handler};
        //auto api_strand = net::make_strand(ioc);


        //http_handler::ApiRequestHandler api_handler{game, static_path, api_strand};
        //http_handler::StaticFileRequestHandler static_file_handler{game, static_path};
        auto api_handler = std::make_shared<http_handler::ApiRequestHandler>(application, static_path);
        auto static_file_handler = std::make_shared<http_handler::StaticFileRequestHandler>(application, static_path);

        http_handler::LoggingRequestHandler<http_handler::ApiRequestHandler> logging_api_handler{*api_handler};
        http_handler::LoggingRequestHandler<http_handler::StaticFileRequestHandler> logging_static_file_handler{*static_file_handler};


        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&logging_api_handler, &logging_static_file_handler](auto&& req, const std::string& client_ip, auto&& send) {
            if (req.target().starts_with("/api/")) {
                logging_api_handler(std::forward<decltype(req)>(req), client_ip, std::forward<decltype(send)>(send));
            } else {
                logging_static_file_handler(std::forward<decltype(req)>(req), client_ip, std::forward<decltype(send)>(send));
            }
        });

        //http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, const std::string& client_ip, auto&& send) {
            //logging_handler(std::forward<decltype(req)>(req), client_ip, std::forward<decltype(send)>(send));
        //});

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы std::cout << "Server has started..."sv << std::endl
        json::value custom_data = json::object{
                {"port"s, port},
                {"address"s, address.to_string()}
        };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "server started"sv;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        json::value custom_data = json::object{
                {"code"s, EXIT_FAILURE},
                {"exception"s, ex.what()}
        };
        BOOST_LOG_TRIVIAL(fatal) << logging::add_value(additional_data, custom_data) << "server exited"sv;
        return EXIT_FAILURE;
    }
}
