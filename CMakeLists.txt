cmake_minimum_required(VERSION 3.29.0)

project(AetherNet
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(AetherNet SHARED
    src/UdpServer.cpp
    src/UdpClient.cpp
    src/AetherNet.cpp
    src/UdpSocket.cpp
    src/SocketUtil.cpp
    src/SocketAddress.cpp
    src/SocketAddressFactory.cpp
    include/UdpServer.h
    include/UdpClient.h
    include/SocketUtil.h
    include/SocketAddress.h
    include/AetherNetExport.h
    include/UdpSocket.h
    include/SocketAddressFactory.h
)

target_include_directories(AetherNet
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(AetherNet
    PRIVATE AETHERNET_EXPORTS
)
