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

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "express/legacy/in/WebApp.h"
#include "express/tls/in/WebApp.h"
#include <SemanticLog.h>

#include <string>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

using namespace express;

int main(int argc, char* argv[]) {
    express::WebApp::init(argc, argv);

    legacy::in::WebApp legacyApp("legacy");

    legacyApp.get("/", [] APPLICATION(req, res) {
        if (req->url == "/" || req->url == "/index.html") {
            req->url = "/wstest.html";
        }

        snode::semantic::appLog().debug() << CMAKE_CURRENT_SOURCE_DIR "/html" + req->url;
        res->sendFile(CMAKE_CURRENT_SOURCE_DIR "/html" + req->url, [&req](int ret) -> void {
            if (ret != 0) {
                snode::semantic::sysError(snode::semantic::appLog(), logger::LogLevel::Error, ret) << req->url;
            }
        });
    });

    legacyApp.get("/ws", [](std::shared_ptr<Request> req, std::shared_ptr<Response> res) -> void {
        std::string uri = req->originalUrl;

        snode::semantic::appLog().debug() << "OriginalUri: " << uri;
        snode::semantic::appLog().debug() << "Uri: " << req->url;

        snode::semantic::appLog().debug() << "Host: " << req->get("host");
        snode::semantic::appLog().debug() << "Connection: " << req->get("connection");
        snode::semantic::appLog().debug() << "Origin: " << req->get("origin");
        snode::semantic::appLog().debug() << "Sec-WebSocket-Protocol: " << req->get("sec-websocket-protocol");
        snode::semantic::appLog().debug() << "sec-web-socket-extensions: " << req->get("sec-websocket-extensions");
        snode::semantic::appLog().debug() << "sec-websocket-key: " << req->get("sec-websocket-key");
        snode::semantic::appLog().debug() << "sec-websocket-version: " << req->get("sec-websocket-version");
        snode::semantic::appLog().debug() << "upgrade: " << req->get("upgrade");
        snode::semantic::appLog().debug() << "user-agent: " << req->get("user-agent");

        if (web::http::ciContains(req->get("connection"), "Upgrade")) {
            res->upgrade(req, [&subProtocolsRequested = req->get("upgrade"), res](const std::string& name) -> void {
                if (!name.empty()) {
                    snode::semantic::appLog().debug() << "Successful upgrade to '" << name << "'  requested: " << subProtocolsRequested;
                } else {
                    snode::semantic::appLog().warn() << "Can not upgrade to any of '" << subProtocolsRequested << "'";
                }
                res->end();
            });
        } else {
            res->sendStatus(404);
        }
    });

    legacyApp.listen([](const legacy::in::WebApp::SocketAddress& socketAddress,
                        const core::socket::State& state) -> void { // Listen on all bluetooth interfaces on channel 16{
        switch (state) {
            case core::socket::State::OK:
                snode::semantic::appLog().info() << "legacy: listening on '" << socketAddress.toString() << "'";
                break;
            case core::socket::State::DISABLED:
                snode::semantic::appLog().info() << "legacy: disabled";
                break;
            case core::socket::State::ERROR:
                snode::semantic::appLog().warn() << "legacy: non critical error occurred";
                break;
            case core::socket::State::FATAL:
                snode::semantic::appLog().critical() << "legacy: critical error occurred";
                break;
        }
    });

    {
        tls::in::WebApp tlsApp("tls");

        tlsApp.get("/", [] APPLICATION(req, res) {
            if (req->url == "/" || req->url == "/index.html") {
                req->url = "/wstest.html";
            }

            snode::semantic::appLog().debug() << CMAKE_CURRENT_SOURCE_DIR "/html" + req->url;
            res->sendFile(CMAKE_CURRENT_SOURCE_DIR "/html" + req->url, [&req](int ret) -> void {
                if (ret != 0) {
                    snode::semantic::sysError(snode::semantic::appLog(), logger::LogLevel::Error, ret) << req->url;
                }
            });
        });

        tlsApp.get("/ws", [](std::shared_ptr<Request> req, std::shared_ptr<Response> res) -> void {
            std::string uri = req->originalUrl;

            snode::semantic::appLog().debug() << "OriginalUri: " << uri;
            snode::semantic::appLog().debug() << "Uri: " << req->url;

            snode::semantic::appLog().debug() << "Connection: " << req->get("connection");
            snode::semantic::appLog().debug() << "Host: " << req->get("host");
            snode::semantic::appLog().debug() << "Origin: " << req->get("origin");
            snode::semantic::appLog().debug() << "Sec-WebSocket-Protocol: " << req->get("sec-websocket-protocol");
            snode::semantic::appLog().debug() << "sec-web-socket-extensions: " << req->get("sec-websocket-extensions");
            snode::semantic::appLog().debug() << "sec-websocket-key: " << req->get("sec-websocket-key");
            snode::semantic::appLog().debug() << "sec-websocket-version: " << req->get("sec-websocket-version");
            snode::semantic::appLog().debug() << "upgrade: " << req->get("upgrade");
            snode::semantic::appLog().debug() << "user-agent: " << req->get("user-agent");

            if (web::http::ciContains(req->get("connection"), "Upgrade")) {
                res->upgrade(req, [&subProtocolsRequested = req->get("upgrade"), res](const std::string& name) -> void {
                    if (!name.empty()) {
                        snode::semantic::appLog().debug() << "Successful upgrade to '" << name << "'  requested: " << subProtocolsRequested;
                    } else {
                        snode::semantic::appLog().warn() << "Can not upgrade to any of '" << subProtocolsRequested << "'";
                    }
                    res->end();
                });
            } else {
                res->sendStatus(404);
            }
        });

        tlsApp.listen([](const legacy::in::WebApp::SocketAddress& socketAddress,
                         const core::socket::State& state) -> void { // Listen on all bluetooth interfaces on channel 16{
            switch (state) {
                case core::socket::State::OK:
                    snode::semantic::appLog().info() << "tls: listening on '" << socketAddress.toString() << "'";
                    break;
                case core::socket::State::DISABLED:
                    snode::semantic::appLog().info() << "tls: disabled";
                    break;
                case core::socket::State::ERROR:
                    snode::semantic::appLog().warn() << "tls: non critical error occurred";
                    break;
                case core::socket::State::FATAL:
                    snode::semantic::appLog().critical() << "tls: critical error occurred";
                    break;
            }
        });
    }

    return express::WebApp::start();
}
