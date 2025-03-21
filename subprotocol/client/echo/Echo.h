/*
 * snode.c - a slim toolkit for network communication
 * Copyright (C) 2020, 2021, 2022 Volker Christian <me@vchrist.at>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WEB_WEBSOCKET_SUBPROTOCOL_ECHO_SERVER_ECHO_H
#define WEB_WEBSOCKET_SUBPROTOCOL_ECHO_SERVER_ECHO_H

#include <web/websocket/client/SubProtocol.h>

namespace web::websocket {
    class SubProtocolContext;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <cstddef> // for std::size_t
#include <cstdint> // for uint16_t
#include <string>  // for string, basic_string

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

namespace web::websocket::subprotocol::echo::client {

    class Echo : public web::websocket::client::SubProtocol {
    private:
        using Super = web::websocket::client::SubProtocol;

    public:
        explicit Echo(SubProtocolContext* subProtocolContext, const std::string& name);
        ~Echo() override = default;

    private:
        void onConnected() override;
        void onMessageStart(int opCode) override;
        void onMessageData(const char* junk, std::size_t junkLen) override;
        void onMessageEnd() override;
        void onMessageError(uint16_t errnum) override;
        void onPongReceived() override;
        void onDisconnected() override;
        [[nodiscard]] bool onSignal(int sig) override;

        std::string data;

        int flyingPings = 0;
    };

} // namespace web::websocket::subprotocol::echo::client

#endif // WEB_WEBSOCKET_SUBPROTOCOL_ECHO_SERVER_ECHO_H
