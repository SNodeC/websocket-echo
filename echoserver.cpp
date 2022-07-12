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
#include "log/Logger.h"
#include "web/http/http_utils.h" // for ci_contains

#include <cerrno>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

using namespace express;

int main(int argc, char* argv[]) {
    express::WebApp::init(argc, argv);

    setenv("DL_WSSERVER_SUBPROTOCOL_PATH", CMAKE_CURRENT_BINARY_DIR "/subprotocol/server/echo", 0);

    legacy::in::WebApp legacyApp("legacy");

    legacyApp.get("/", [] APPLICATION(req, res) {
        if (req.url == "/" || req.url == "/index.html") {
            req.url = "/wstest.html";
        }

        VLOG(0) << CMAKE_CURRENT_SOURCE_DIR "/html" + req.url;
        res.sendFile(CMAKE_CURRENT_SOURCE_DIR "/html" + req.url, [&req](int ret) -> void {
            if (ret != 0) {
                PLOG(ERROR) << req.url;
            }
        });
    });

    legacyApp.get("/ws", [](Request& req, Response& res) -> void {
        std::string uri = req.originalUrl;

        VLOG(1) << "OriginalUri: " << uri;
        VLOG(1) << "Uri: " << req.url;

        VLOG(1) << "Host: " << req.get("host");
        VLOG(1) << "Connection: " << req.get("connection");
        VLOG(1) << "Origin: " << req.get("origin");
        VLOG(1) << "Sec-WebSocket-Protocol: " << req.get("sec-websocket-protocol");
        VLOG(1) << "sec-web-socket-extensions: " << req.get("sec-websocket-extensions");
        VLOG(1) << "sec-websocket-key: " << req.get("sec-websocket-key");
        VLOG(1) << "sec-websocket-version: " << req.get("sec-websocket-version");
        VLOG(1) << "upgrade: " << req.get("upgrade");
        VLOG(1) << "user-agent: " << req.get("user-agent");

        if (httputils::ci_contains(req.get("connection"), "Upgrade")) {
            res.upgrade(req);
        } else {
            res.sendStatus(404);
        }
    });

    legacyApp.listen([](const tls::in::WebApp::SocketAddress& socketAddress, int errnum) -> void {
        if (errnum < 0) {
            PLOG(ERROR) << "OnError";
        } else if (errnum > 0) {
            errno = errnum;
            PLOG(ERROR) << "OnError: " << socketAddress.toString();
        } else {
            VLOG(0) << "snode.c connecting to " << socketAddress.toString();
        }
    });

    std::map<std::string, std::any> options;
    options["CertChain"] = CMAKE_CURRENT_SOURCE_DIR "/certs/WebServerCertificateChain.pem";
    options["CertChainKey"] = CMAKE_CURRENT_SOURCE_DIR "/certs/Volker_Christian_-_WEB-Cert.pem";
    options["Password"] = "pentium5";

    {
        tls::in::WebApp tlsApp("tls", options);

        tlsApp.get("/", [] APPLICATION(req, res) {
            if (req.url == "/" || req.url == "/index.html") {
                req.url = "/wstest.html";
            }

            VLOG(0) << CMAKE_CURRENT_SOURCE_DIR "/html" + req.url;
            res.sendFile(CMAKE_CURRENT_SOURCE_DIR "/html" + req.url, [&req](int ret) -> void {
                if (ret != 0) {
                    PLOG(ERROR) << req.url;
                }
            });
        });

        tlsApp.get("/ws", [](Request& req, Response& res) -> void {
            std::string uri = req.originalUrl;

            VLOG(1) << "OriginalUri: " << uri;
            VLOG(1) << "Uri: " << req.url;

            VLOG(1) << "Connection: " << req.get("connection");
            VLOG(1) << "Host: " << req.get("host");
            VLOG(1) << "Origin: " << req.get("origin");
            VLOG(1) << "Sec-WebSocket-Protocol: " << req.get("sec-websocket-protocol");
            VLOG(1) << "sec-web-socket-extensions: " << req.get("sec-websocket-extensions");
            VLOG(1) << "sec-websocket-key: " << req.get("sec-websocket-key");
            VLOG(1) << "sec-websocket-version: " << req.get("sec-websocket-version");
            VLOG(1) << "upgrade: " << req.get("upgrade");
            VLOG(1) << "user-agent: " << req.get("user-agent");

            if (httputils::ci_contains(req.get("connection"), "Upgrade")) {
                res.upgrade(req);
            } else {
                res.sendStatus(404);
            }
        });

        tlsApp.listen([](const tls::in::WebApp::SocketAddress& socketAddress, int errnum) -> void {
            if (errnum < 0) {
                PLOG(ERROR) << "OnError";
            } else if (errnum > 0) {
                errno = errnum;
                PLOG(ERROR) << "OnError: " << socketAddress.toString();
            } else {
                VLOG(0) << "snode.c connecting to " << socketAddress.toString();
            }
        });
    }

    return express::WebApp::start();
}
