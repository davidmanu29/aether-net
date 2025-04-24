cmake_minimum_required(VERSION 3.29.0)

project(AetherNet
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(AetherNet SHARED
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

target_link_libraries(AetherNet PRIVATE ws2_32)

add_executable(UdpServerExe src/UdpServer.cpp)
target_link_libraries(UdpServerExe PRIVATE AetherNet ws2_32)
target_compile_definitions(UdpServerExe PRIVATE UDPSERVER_MAIN)

add_executable(UdpClientExe src/UdpClient.cpp)
target_link_libraries(UdpClientExe PRIVATE AetherNet ws2_32)
target_compile_definitions(UdpClientExe PRIVATE UDPCLIENT_MAIN)