project(haxxerz-chat)
cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON )

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system)


include_directories(
    ${Boost_INCLUDE_DIR}
    )

include_directories(include
    third-party)

set(SRC_SRV src/server/server.cpp
    src/server/client.cpp
    src/server/main.cpp)

set(SRC_CLI src/client/client.cpp
    src/client/main.cpp
    src/client/ui.cpp
    third-party/imgui/imgui.cpp
    third-party/imgui/imgui_draw.cpp
    third-party/imgui/imgui_widgets.cpp
    third-party/imgui/imgui_demo.cpp
    third-party/imgui/examples/imgui_impl_opengl3.cpp
    third-party/imgui/examples/imgui_impl_sdl.cpp)

add_executable(server ${SRC_SRV})
target_include_directories(server PRIVATE include
    include/server
    ${Boost_INCLUDE_DIR})

add_executable(client ${SRC_CLI})
target_include_directories(client PRIVATE include
    include/client
    third-party/imgui
    third-party/rwqueue
    third-party/imgui/examples
    ${Boost_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    )

target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    cryptopp)
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
    cryptopp)
