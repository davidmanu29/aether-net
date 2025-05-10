cmake_minimum_required(VERSION 3.29.0)

project(AetherNet
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

add_library(AetherNet SHARED
    src/UdpClient.cpp
    src/AetherNet.cpp
    src/UdpSocket.cpp
    src/UdpServer.cpp
    src/SocketUtil.cpp
    src/SocketAddress.cpp
    src/SocketAddressFactory.cpp
    src/NatPuncher.cpp
    include/UdpServer.h
    include/NatPuncher.h
    include/UdpClient.h
    include/UdpServer.h
    include/SocketUtil.h
    include/SocketAddress.h
    include/AetherNetExport.h
    include/UdpSocket.h
    include/SocketAddressFactory.h
    include/WinsockInitializer.h
)

target_include_directories(AetherNet
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(AetherNet
    PRIVATE AETHERNET_EXPORTS
)

target_link_libraries(AetherNet PRIVATE ws2_32)

add_executable(UdpServerExe src/server_main.cpp)
target_link_libraries(UdpServerExe PRIVATE AetherNet ws2_32)
target_compile_definitions(UdpServerExe PRIVATE UDPSERVER_MAIN)

find_package(OpenGL REQUIRED)

add_executable(UdpClientExe 
    ClientDemo/main.cpp
    ClientDemo/GameClient.cpp
    ClientDemo/GameClient.h
    ClientDemo/UdpProtocol.h
)

target_include_directories(UdpClientExe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ClientDemo
)
target_link_libraries(UdpClientExe PRIVATE AetherNet ws2_32 glfw OpenGL::GL)
target_compile_definitions(UdpClientExe PRIVATE UDPCLIENT_MAIN)