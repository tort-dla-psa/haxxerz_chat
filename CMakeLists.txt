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

# execute_process (
#     COMMAND bash -c "wget -O CMakeLists.txt https://raw.githubusercontent.com/noloader/cryptopp-cmake/master/CMakeLists.txt &&
#         wget -O cryptopp-config.cmake https://raw.githubusercontent.com/noloader/cryptopp-cmake/master/cryptopp-config.cmake
#         "
#     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third-party/cryptopp
#     OUTPUT_VARIABLE wget_success
# )

# set(BUILD_TESTING OFF CACHE BOOL "enable cryptopp tests" FORCE)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third-party/cryptopp)

include_directories(include
    include/core
    third-party)

set(SRC_ENC 
    # example/src/encrypt_engine/enc_eng.cpp
    )

set(SRC_SRV 
    src/core/server/server.cpp
    src/core/server/client.cpp
    example/src/server/chat_server.cpp
    example/src/server/chat_client.cpp
    example/src/server/main.cpp
    ${SRC_ENC}
    )

set(SRC_CLI 
    src/core/client/client.cpp
    example/src/client/chat_client.cpp
    example/src/client/main.cpp
    example/src/client/ui.cpp
    third-party/imgui/imgui.cpp
    third-party/imgui/imgui_draw.cpp
    third-party/imgui/imgui_widgets.cpp
    third-party/imgui/imgui_demo.cpp
    third-party/imgui/examples/imgui_impl_opengl3.cpp
    third-party/imgui/examples/imgui_impl_sdl.cpp
    ${SRC_ENC}
    )

add_executable(server ${SRC_SRV})
target_include_directories(server PRIVATE include
    example/include/server
    example/include/
    ${Boost_INCLUDE_DIR})

add_executable(client ${SRC_CLI})
target_include_directories(client PRIVATE include
    example/include/
    example/include/client
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
#     cryptopp
    )
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
#     cryptopp
    )
