cmake_minimum_required(VERSION 3.29.0)

project(AetherNet
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

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

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

set(INSTALL_GTEST  OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK    OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

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
    ClientDemo/SecurityService.h
)

target_include_directories(UdpClientExe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ClientDemo
)
target_link_libraries(UdpClientExe PRIVATE AetherNet ws2_32 glfw OpenGL::GL)
target_compile_definitions(UdpClientExe PRIVATE UDPCLIENT_MAIN)

add_executable(AetherNet_tests
    tests/SocketAddress_test.cpp
    tests/UdpSocket_test.cpp
)
target_include_directories(AetherNet_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(AetherNet_tests
    PRIVATE AetherNet GTest::gtest_main ws2_32
)
include(GoogleTest)
gtest_discover_tests(AetherNet_tests
    PROPERTIES LABELS unit
)