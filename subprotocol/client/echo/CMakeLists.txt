cmake_minimum_required(VERSION 3.5)

find_package(snodec COMPONENTS websocket-client)

set(ECHOCLIENTSUBPROTOCOL_CPP Echo.cpp EchoFactory.cpp)

set(ECHOCLIENTSUBPROTOCOL_H Echo.h EchoFactory.h)

add_library(
    echoclientsubprotocol SHARED ${ECHOCLIENTSUBPROTOCOL_CPP}
                                 ${ECHOCLIENTSUBPROTOCOL_H}
)

target_link_libraries(echoclientsubprotocol PUBLIC snodec::websocket-client)

set_target_properties(
    echoclientsubprotocol
    PROPERTIES OUTPUT_NAME "snodec-websocket-echo"
               SOVERSION 1 # PREFIX "ssp"
)

install(TARGETS echoclientsubprotocol
        LIBRARY DESTINATION ${WEBSOCKET_SUBPROTOCOL_INSTALL_LIBDIR}
)

if(CHECK_INCLUDES)
    set_property(
        TARGET echoclientsubprotocol PROPERTY CXX_INCLUDE_WHAT_YOU_USE
                                              ${iwyu_path_and_options}
    )
endif(CHECK_INCLUDES)
