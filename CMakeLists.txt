project(haxxerz-chat)
cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system)

include_directories(${Boost_INCLUDE_DIR})

include_directories(include)

set(SRC_SRV src/server/server.cpp
    src/server/client.cpp
    src/server/main.cpp)

set(CLI_SRV src/client/client.cpp
    src/client/main.cpp)

add_executable(server ${SRC_SRV})
target_include_directories(server PRIVATE include
    include/server
    ${Boost_INCLUDE_DIR})

add_executable(client ${CLI_SRV})
target_include_directories(client PRIVATE include
    include/client
    ${Boost_INCLUDE_DIR})

target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT}
    ${CURSES_LIBRARIES}
    ${Boost_LIBRARIES}
    cryptopp)
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT}
    ${CURSES_LIBRARIES}
    ${Boost_LIBRARIES}
    cryptopp)
