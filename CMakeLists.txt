cmake_minimum_required(VERSION 3.5)

project(WebSocket-Echo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(snodec COMPONENTS express httpclient net-in core-legacy core-tls)

set(WSECHOSERVER_CPP echoserver.cpp)

set(WSECHOSERVER_H)

add_executable(wsechoserver ${WSECHOSERVER_CPP} ${WSECHOSERVER_H})
target_compile_definitions(
    wsechoserver PRIVATE CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}" CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(
    wsechoserver PRIVATE snodec::express snodec::core-legacy snodec::core-tls
                         snodec::net-in
)
install(TARGETS wsechoserver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set(WSECHOCLIENT_CPP echoclient.cpp)

set(WSECHOCLIENT_H)

add_executable(wsechoclient ${WSECHOCLIENT_CPP} ${WSECHOCLIENT_H})
target_compile_definitions(
    wsechoclient PRIVATE CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(
    wsechoclient PRIVATE snodec::httpclient snodec::core-legacy
                         snodec::core-tls snodec::net-in
)
install(TARGETS wsechoclient RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(subprotocol)