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

add_library(GameStaticLib  STATIC
        src/model/maps.cpp
        src/model/game.cpp
        src/model/dog.cpp
        src/model/model.cpp
        src/model/loot_generator.cpp
        src/model/lost_object.cpp
        src/model/game_session.cpp
)
target_include_directories(GameStaticLib PUBLIC
        ${ZLIB_INCLUDES}
        src/model
)
target_link_libraries(GameStaticLib PUBLIC ${BOOST_LIB} ${ZLIB_LIB})

add_executable(game_server
    src/main.cpp
    src/http_server/http_server.cpp
    src/sdk.h
    src/app/players.cpp
    src/app/player_tokens.cpp
    src/app/application.cpp  
    src/request_handler/api_request_handler.cpp
    src/json_loader/boost_json.cpp
    src/json_loader/json_loader.cpp
    src/files.cpp
    src/logger/logger.cpp
    src/time/ticker.cpp
    src/program_options/program_options.cpp
)

target_include_directories(game_server PRIVATE
    CONAN_PKG::boost
    src
    src/http_server
    src/app
    src/model
    src/request_handler
    src/json_loader
    src/logger
    src/time
    src/program_options
)
target_link_libraries(game_server CONAN_PKG::boost Threads::Threads GameStaticLib)

include(CTest)
include(${CONAN_BUILD_DIRS_CATCH2}/Catch.cmake)

add_executable(game_server_tests
    tests/loot_generator_tests.cpp
)
target_include_directories(game_server_tests PRIVATE  CONAN_PKG::boost)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2
                        CONAN_PKG::boost Threads::Threads GameStaticLib)

catch_discover_tests(game_server_tests)
