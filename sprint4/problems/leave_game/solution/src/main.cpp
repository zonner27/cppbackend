#include "sdk.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <thread>

#include "json_loader/json_loader.h"
#include "request_handler/request_handler.h"
#include "request_handler/api_request_handler.h"
#include "request_handler/logging_request_handler.h"
#include "request_handler/static_request_handler.h"
#include "files.h"
#include "logger/logger.h"
#include "app/players.h"
#include "app/application.h"
#include "program_options/program_options.h"
#include "database/db_settings.h"

constexpr const char DB_URL_ENV_NAME[]{"GAME_DB_URL"};

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

    LoggerInit();

    try {
        auto args = ParseCommandLine(argc, argv);
        const unsigned num_threads = std::thread::hardware_concurrency();
        if (!args) {
            return EXIT_SUCCESS;
        }

        fs::path base_path = args->base_path.string();
        base_path = base_path.parent_path() / "../../"s;
        fs::path json_path = args->config_file;
        fs::path static_path = args->www_root;
        if (!files_path::IsSubPath(static_path, base_path)) {
            throw std::runtime_error("The folder static_path is not in the program directory"s);
        }

        // 1. Получаем из переменной окружения db_url
        const char* db_url = std::getenv(DB_URL_ENV_NAME);
        if (!db_url) {
            throw std::runtime_error(DB_URL_ENV_NAME + " environment variable not found"s);
        }
        db_app::DBSettings db_settings{num_threads, std::move(db_url)};

        // 2. Загружаем карту из файла и построить модель игры
        model::Game game;
        try {
            game = json_loader::LoadGame(json_path);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }

        // 3. Инициализируем io_context
        net::io_context ioc(num_threads);
        auto application = std::make_shared<app::Application>(game, ioc, args->tick_period,
                                                              args->randomize_spawn_points,
                                                              db_settings);

        // 4. Загрузка сохраненной игры
        if (!args->state_file.empty()) {
            application->LoadGame(args->state_file, std::chrono::milliseconds(args->save_state_period));
        }

        // 5. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        // Подписываемся на сигналы и при их получении завершаем работу сервера
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, &application](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {

                json::value custom_data = json::object{{"code"s, 0}};
                BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "server exited"sv;
                application->SaveGame();
                ioc.stop();
            }
        });

        // 6. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        auto api_handler = std::make_shared<http_handler::ApiRequestHandler>(*application, static_path);
        auto static_file_handler = std::make_shared<http_handler::StaticFileRequestHandler>(*application, static_path);

        http_handler::LoggingRequestHandler<http_handler::ApiRequestHandler> logging_api_handler{*api_handler};
        http_handler::LoggingRequestHandler<http_handler::StaticFileRequestHandler> logging_static_file_handler{*static_file_handler};

        // 7. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&logging_api_handler, &logging_static_file_handler](auto&& req, const std::string& client_ip, auto&& send) {
            if (req.target().starts_with("/api/")) {
                logging_api_handler(std::forward<decltype(req)>(req), client_ip, std::forward<decltype(send)>(send));
            } else {
                logging_static_file_handler(std::forward<decltype(req)>(req), client_ip, std::forward<decltype(send)>(send));
            }
        });

        json::value custom_data = json::object{
                {"port"s, port},
                {"address"s, address.to_string()}
        };
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, custom_data) << "server started"sv;

        // 8. Запускаем обработку асинхронных операций
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
