cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

# Флаг -DNDEBUG для релизной сборки
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(game_server
	src/main.cpp
        src/http_server/http_server.cpp
        src/http_server/http_server.h
	src/sdk.h

        src/app/players.h
        src/app/players.cpp
        src/app/player_tokens.h
        src/app/player_tokens.cpp
        src/app/application.h
        src/app/application.cpp


        src/model/model.h
        src/model/model.cpp
        src/model/dog.h
        src/model/dog.cpp
        src/model/game.h
        src/model/game.cpp
        src/model/game_session.h
        src/model/game_session.cpp
        src/model/maps.h
        src/model/maps.cpp
        src/model/loot_generator.h
        src/model/loot_generator.cpp
        src/model/lost_object.h
        src/model/lost_object.cpp

        src/request_handler/request_handler.h
        src/request_handler/api_request_handler.cpp
        src/request_handler/api_request_handler.h
        src/request_handler/static_request_handler.h
        src/request_handler/logging_request_handler.h

	src/tagged.h
        src/json_loader/boost_json.cpp
        src/json_loader/json_loader.h
        src/json_loader/json_loader.cpp        
	src/constants.h
        src/files.cpp
        src/files.h
        src/streamadapter.h
        src/logger/logger.h
        src/logger/logger.cpp

        src/time/ticker.h
        src/time/ticker.cpp

        src/program_options/program_options.h
        src/program_options/program_options.cpp


)
target_include_directories(game_server PRIVATE CONAN_PKG::boost)
target_link_libraries(game_server CONAN_PKG::boost Threads::Threads)

add_executable(game_server_tests
    tests/loot_generator_tests.cpp
    src/model/loot_generator.h
    src/model/loot_generator.cpp
)
target_include_directories(game_server_tests PRIVATE  CONAN_PKG::boost)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2
                                CONAN_PKG::boost Threads::Threads)


