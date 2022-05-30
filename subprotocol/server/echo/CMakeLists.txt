cmake_minimum_required(VERSION 3.5)

find_package(snodec COMPONENTS websocketserver)

get_target_property(
    WEBSOCKET_SUBPROTOCOL_SERVER_INSTALL_LIBDIR snodec::websocketserver
    WEBSOCKET_SUBPROTOCOL_SERVER_INSTALL_LIBDIR
)

get_target_property(
    WEBSOCKET_SUBPROTOCOL_SERVER_INCLUDE_DIRS snodec::websocketserver
    WEBSOCKET_SUBPROTOCOL_SERVER_INCLUDE_DIRS
)

set(ECHOSERVERSUBPROTOCOL_CPP Echo.cpp EchoFactory.cpp)

set(ECHOSERVERSUBPROTOCOL_H Echo.h EchoFactory.h)

add_library(
    echoserversubprotocol SHARED ${ECHOSERVERSUBPROTOCOL_CPP}
                                 ${ECHOSERVERSUBPROTOCOL_H}
)

target_include_directories(
    echoserversubprotocol PUBLIC ${WEBSOCKET_SUBPROTOCOL_SERVER_INCLUDE_DIRS}
)

set_target_properties(
    echoserversubprotocol
    PROPERTIES OUTPUT_NAME "snodec-websocket-echo"
               SOVERSION 1 # PREFIX "ssp"
)

install(TARGETS echoserversubprotocol
        LIBRARY DESTINATION ${WEBSOCKET_SUBPROTOCOL_SERVER_INSTALL_LIBDIR}
)

if(CHECK_INCLUDES)
    set_property(
        TARGET echoserversubprotocol PROPERTY CXX_INCLUDE_WHAT_YOU_USE
                                              ${iwyu_path_and_options}
    )
endif(CHECK_INCLUDES)
