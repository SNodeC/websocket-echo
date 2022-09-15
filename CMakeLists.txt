cmake_minimum_required(VERSION 3.5)

project(WebSocket-Echo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(snodec COMPONENTS http-server-express http-client net-in-stream-legacy net-in-stream-tls)

find_program(iwyu_path NAMES include-what-you-use iwyu)

if(iwyu_path)
    option(CHECK_INCLUDES "Check used headers")

    set(iwyu_path_and_options
        ${iwyu_path}
        -Xiwyu
        --verbose=3
        -Xiwyu
        --cxx17ns
        -Xiwyu
        --quoted_includes_first
        -Xiwyu
        --check_also='${PROJECT_SOURCE_DIR}/*'
    )

    if(CHECK_INCLUDES)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path_and_options})
    endif(CHECK_INCLUDES)
else()
    message(
        AUTHOR_WARNING "Could not find the program include-what-you-use"
    )
endif()

set(WSECHOSERVER_CPP echoserver.cpp)

set(WSECHOSERVER_H)

add_executable(wsechoserver ${WSECHOSERVER_CPP} ${WSECHOSERVER_H})
target_compile_definitions(
    wsechoserver PRIVATE CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
                         CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(
    wsechoserver PRIVATE snodec::http-server-express snodec::net-in-stream-legacy snodec::net-in-stream-tls
)
install(TARGETS wsechoserver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set(WSECHOCLIENT_CPP echoclient.cpp)

set(WSECHOCLIENT_H)

add_executable(wsechoclient ${WSECHOCLIENT_CPP} ${WSECHOCLIENT_H})
target_compile_definitions(
    wsechoclient PRIVATE CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(
    wsechoclient PRIVATE snodec::http-client snodec::net-in-stream-legacy snodec::net-in-stream-tls
)
install(TARGETS wsechoclient RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(subprotocol)
