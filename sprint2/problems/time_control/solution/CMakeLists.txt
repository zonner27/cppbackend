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
        src/model/model.h
        src/model/model.cpp
	src/tagged.h
        src/json_loader/boost_json.cpp
        src/json_loader/json_loader.h
        src/json_loader/json_loader.cpp
        src/request_handler/request_handler.cpp
        src/request_handler/request_handler.h
	src/constants.h
        src/files.cpp
        src/files.h
        src/streamadapter.h
        src/logger/logger.h
        src/logger/logger.cpp
        src/app/players.h
        src/app/players.cpp
)
target_include_directories(game_server PRIVATE CONAN_PKG::boost)
target_link_libraries(game_server 			
				CONAN_PKG::boost
				Threads::Threads)




