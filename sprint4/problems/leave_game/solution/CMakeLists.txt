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

add_library(collision_detection_lib STATIC
        src/events/collision_detector.h
        src/events/collision_detector.cpp
        src/events/geom.h
)
target_link_libraries(collision_detection_lib PUBLIC CONAN_PKG::boost Threads::Threads)

add_library(GameStaticLib  STATIC
        src/model/maps.cpp
        src/model/maps.h
        src/model/game.cpp
        src/model/game.h
        src/model/dog.cpp
        src/model/dog.h
        src/model/model.cpp
        src/model/model.h
        src/model/loot_generator.cpp
        src/model/loot_generator.h
        src/model/lost_object.cpp
        src/model/lost_object.h
        src/model/game_session.cpp
        src/model/game_session.h
        src/model/item_gatherer_provider.h
        src/model/item_gatherer_provider.cpp        
)
target_include_directories(GameStaticLib PUBLIC src/model)
target_link_libraries(GameStaticLib PUBLIC ${BOOST_LIB} collision_detection_lib)

add_executable(game_server
    src/main.cpp
    src/sdk.h
    src/streamadapter.h
    src/constants.h
    src/tagged.h
    src/tagged_uuid.h
    src/tagged_uuid.cpp
    src/files.cpp
    src/files.h

    src/http_server/http_server.cpp
    src/http_server/http_server.h

    src/app/players.cpp
    src/app/players.h
    src/app/player_tokens.cpp
    src/app/player_tokens.h
    src/app/application.cpp  
    src/app/application.h

    src/request_handler/api_request_handler.cpp
    src/request_handler/api_request_handler.h
    src/request_handler/logging_request_handler.h
    src/request_handler/request_handler.h
    src/request_handler/static_request_handler.h

    src/json_loader/boost_json.cpp
    src/json_loader/json_loader.cpp
    src/json_loader/json_loader.h

    src/logger/logger.cpp
    src/logger/logger.h

    src/time/ticker.cpp
    src/time/ticker.h

    src/program_options/program_options.cpp
    src/program_options/program_options.h

    src/serialization/model_serialization.h
    src/serialization/dog_serialization.h
    src/serialization/dog_serialization.cpp
    src/serialization/lost_object_serialization.h
    src/serialization/lost_object_serialization.cpp
    src/serialization/game_session_serialization.h
    src/serialization/game_session_serialization.cpp
    src/serialization/player_serialization.h
    src/serialization/player_serialization.cpp

    src/database/connection_pool.h
    src/database/db_settings.h
    src/database/postgres.cpp
    src/database/postgres.h
    src/database/retired_players.h
    src/database/retired_players.cpp
    src/database/retired_players_fwd.h
    src/database/use_cases.h
    src/database/use_cases_impl.h
    src/database/use_cases_impl.cpp
)

target_include_directories(game_server PRIVATE
    src
    src/http_server
    src/app
    src/model
    src/request_handler
    src/json_loader
    src/logger
    src/time
    src/program_options
    src/events
    src/serialization
    src/database
)
target_link_libraries(game_server CONAN_PKG::boost Threads::Threads
                    CONAN_PKG::libpq CONAN_PKG::libpqxx GameStaticLib)

# Tests
include(CTest)
include(${CONAN_BUILD_DIRS_CATCH2}/Catch.cmake)

add_executable(game_server_tests tests/loot_generator_tests.cpp)
target_include_directories(game_server_tests PRIVATE  CONAN_PKG::boost)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 CONAN_PKG::boost Threads::Threads GameStaticLib)

add_executable(collision_detection_tests tests/collision-detector-tests.cpp)
target_link_libraries(collision_detection_tests CONAN_PKG::catch2 collision_detection_lib)

add_executable(state_serialization_tests tests/state-serialization-tests.cpp
                                        src/serialization/lost_object_serialization.h
                                        src/serialization/lost_object_serialization.cpp)
target_link_libraries(state_serialization_tests CONAN_PKG::catch2 collision_detection_lib GameStaticLib)

catch_discover_tests(game_server_tests)
catch_discover_tests(collision_detection_tests)
catch_discover_tests(state_serialization_tests)

