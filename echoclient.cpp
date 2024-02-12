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

#include "core/SNodeC.h"               // for SNodeC
#include "log/Logger.h"                // for Writer, Storage
#include "web/http/legacy/in/Client.h" // for Client, Client<>...
#include "web/http/tls/in/Client.h"    // for Client, Client<>...

#include <cstdlib>
#include <openssl/ssl.h> // IWYU pragma: keep
#include <openssl/x509v3.h>
#include <string>
#include <utility>

// IWYU pragma: no_include <openssl/asn1.h>
// IWYU pragma: no_include <openssl/crypto.h>
// IWYU pragma: no_include <openssl/obj_mac.h>
// IWYU pragma: no_include <openssl/ssl3.h>
// IWYU pragma: no_include <openssl/types.h>
// IWYU pragma: no_include <openssl/x509.h>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

int main(int argc, char* argv[]) {
    core::SNodeC::init(argc, argv);

    setenv("DL_WSCLIENT_SUBPROTOCOL_PATH", CMAKE_CURRENT_BINARY_DIR "/subprotocol/client/echo", 0);

    {
        using EchoClientLegacy = web::http::legacy::in::Client;
        using SocketConnectionLegacy = EchoClientLegacy::SocketConnection;
        using Request = EchoClientLegacy::Request;
        using Response = EchoClientLegacy::Response;
        using SocketAddressLegacy = EchoClientLegacy::SocketAddress;

        EchoClientLegacy legacyClient(
            "legacy",
            [](SocketConnectionLegacy* socketConnection) -> void {
                VLOG(0) << "OnConnect";

                VLOG(0) << "\tServer: " + socketConnection->getRemoteAddress().toString();
                VLOG(0) << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            []([[maybe_unused]] SocketConnectionLegacy* socketConnection) -> void {
                VLOG(0) << "OnConnected";
            },
            [](std::shared_ptr<Request>& request) -> void {
                VLOG(0) << "OnRequestBegin";

                request->set("Sec-WebSocket-Protocol", "test, echo");

                request->upgrade("/ws/", "websocket");
            },
            [](std::shared_ptr<Request>& request, std::shared_ptr<Response>& response) -> void {
                VLOG(0) << "OnResponse";
                VLOG(0) << "     Status:";
                VLOG(0) << "       " << response->httpVersion << " " << response->statusCode << " " << response->reason;

                VLOG(0) << "     Headers:";
                for (const auto& [field, value] : response->headers) {
                    VLOG(0) << "       " << field + " = " + value;
                }

                VLOG(0) << "     Cookies:";
                for (const auto& [name, cookie] : response->cookies) {
                    VLOG(0) << "       " + name + " = " + cookie.getValue();
                    for (const auto& [option, value] : cookie.getOptions()) {
                        VLOG(0) << "         " + option + " = " + value;
                    }
                }

                response->body.push_back(0); // make it a c-string
                VLOG(0) << "Body:\n----------- start body -----------\n" << response->body.data() << "\n------------ end body ------------";

                request->upgrade(response);
            },
            [](int status, const std::string& reason) -> void {
                VLOG(0) << "OnResponseError";
                VLOG(0) << "     Status: " << status;
                VLOG(0) << "     Reason: " << reason;
            },
            [](SocketConnectionLegacy* socketConnection) -> void {
                VLOG(0) << "OnDisconnect";

                VLOG(0) << "\tServer: " + socketConnection->getRemoteAddress().toString();
                VLOG(0) << "\tClient: " + socketConnection->getLocalAddress().toString();
            });

        legacyClient.connect("localhost", 8080, [](const SocketAddressLegacy& socketAddress, const core::socket::State& state) -> void {
            switch (state) {
                case core::socket::State::OK:
                    VLOG(1) << "legacy: connected to '" << socketAddress.toString() << "': " << state.what();
                    break;
                case core::socket::State::DISABLED:
                    VLOG(1) << "legacy: disabled";
                    break;
                case core::socket::State::ERROR:
                    VLOG(1) << "legacy: " << socketAddress.toString() << ": non critical error occurred";
                    VLOG(1) << "    " << state.what();
                    break;
                case core::socket::State::FATAL:
                    VLOG(1) << "legacy: " << socketAddress.toString() << ": critical error occurred";
                    VLOG(1) << "    " << state.what();
                    break;
            }
        }); // Connection:keep-alive\r\n\r\n"

        using EchoClientTls = web::http::tls::in::Client;
        using SocketConnectionTLS = EchoClientTls::SocketConnection;
        using Request = EchoClientTls::Request;
        using Response = EchoClientTls::Response;
        using SocketAddressTLS = EchoClientTls::SocketAddress;

        EchoClientTls tlsClient(
            "tls",
            [](SocketConnectionTLS* socketConnection) -> void {
                VLOG(0) << "OnConnect";

                VLOG(0) << "\tServer: " + socketConnection->getRemoteAddress().toString();
                VLOG(0) << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            [](SocketConnectionTLS* socketConnection) -> void {
                VLOG(0) << "OnConnected";
                X509* server_cert = SSL_get_peer_certificate(socketConnection->getSSL());
                if (server_cert != nullptr) {
                    long verifyErr = SSL_get_verify_result(socketConnection->getSSL());

                    VLOG(0) << "     Server certificate: " + std::string(X509_verify_cert_error_string(verifyErr));

                    char* str = X509_NAME_oneline(X509_get_subject_name(server_cert), nullptr, 0);
                    VLOG(0) << "        Subject: " + std::string(str);
                    OPENSSL_free(str);

                    str = X509_NAME_oneline(X509_get_issuer_name(server_cert), nullptr, 0);
                    VLOG(0) << "        Issuer: " + std::string(str);
                    OPENSSL_free(str);

                    // We could do all sorts of certificate verification stuff here
                    // before deallocating the certificate.

                    GENERAL_NAMES* subjectAltNames =
                        static_cast<GENERAL_NAMES*>(X509_get_ext_d2i(server_cert, NID_subject_alt_name, nullptr, nullptr));

                    int32_t altNameCount = sk_GENERAL_NAME_num(subjectAltNames);
                    VLOG(0) << "        Subject alternative name count: " << altNameCount;
                    for (int32_t i = 0; i < altNameCount; ++i) {
                        GENERAL_NAME* generalName = sk_GENERAL_NAME_value(subjectAltNames, i);
                        if (generalName->type == GEN_URI) {
                            std::string subjectAltName =
                                std::string(reinterpret_cast<const char*>(ASN1_STRING_get0_data(generalName->d.uniformResourceIdentifier)),
                                            static_cast<std::size_t>(ASN1_STRING_length(generalName->d.uniformResourceIdentifier)));
                            VLOG(0) << "           SAN (URI): '" + subjectAltName;
                        } else if (generalName->type == GEN_DNS) {
                            std::string subjectAltName =
                                std::string(reinterpret_cast<const char*>(ASN1_STRING_get0_data(generalName->d.dNSName)),
                                            static_cast<std::size_t>(ASN1_STRING_length(generalName->d.dNSName)));
                            VLOG(0) << "           SAN (DNS): '" + subjectAltName;
                        } else {
                            VLOG(0) << "           SAN (Type): '" + std::to_string(generalName->type);
                        }
                    }
                    sk_GENERAL_NAME_pop_free(subjectAltNames, GENERAL_NAME_free);

                    X509_free(server_cert);
                } else {
                    VLOG(0) << "     Server certificate: no certificate";
                }
            },
            [](std::shared_ptr<Request>& request) -> void {
                VLOG(0) << "OnRequestBegin";

                request->set("Sec-WebSocket-Protocol", "test, echo");

                request->upgrade("/ws/", "websocket");
            },
            [](std::shared_ptr<Request>& request, std::shared_ptr<Response>& response) -> void {
                VLOG(0) << "OnResponse";
                VLOG(0) << "     Status:";
                VLOG(0) << "       " << response->httpVersion << " " << response->statusCode << " " << response->reason;

                VLOG(0) << "     Headers:";
                for (const auto& [field, value] : response->headers) {
                    VLOG(0) << "       " << field + " = " + value;
                }

                VLOG(0) << "     Cookies:";
                for (const auto& [name, cookie] : response->cookies) {
                    VLOG(0) << "       " + name + " = " + cookie.getValue();
                    for (const auto& [option, value] : cookie.getOptions()) {
                        VLOG(0) << "         " + option + " = " + value;
                    }
                }

                response->body.push_back(0); // make it a c-string
                VLOG(0) << "Body:\n----------- start body -----------\n" << response->body.data() << "\n------------ end body ------------";

                request->upgrade(response);
            },
            [](int status, const std::string& reason) -> void {
                VLOG(0) << "OnResponseError";
                VLOG(0) << "     Status: " << status;
                VLOG(0) << "     Reason: " << reason;
            },
            [](SocketConnectionTLS* socketConnection) -> void {
                VLOG(0) << "OnDisconnect";

                VLOG(0) << "\tServer: " + socketConnection->getRemoteAddress().toString();
                VLOG(0) << "\tClient: " + socketConnection->getLocalAddress().toString();
            });

        tlsClient.connect("localhost", 8088, [](const SocketAddressTLS& socketAddress, const core::socket::State& state) -> void {
            switch (state) {
                case core::socket::State::OK:
                    VLOG(1) << "legacy: connected to '" << socketAddress.toString() << "': " << state.what();
                    break;
                case core::socket::State::DISABLED:
                    VLOG(1) << "legacy: disabled";
                    break;
                case core::socket::State::ERROR:
                    VLOG(1) << "legacy: " << socketAddress.toString() << ": non critical error occurred";
                    VLOG(1) << "    " << state.what();
                    break;
                case core::socket::State::FATAL:
                    VLOG(1) << "legacy: " << socketAddress.toString() << ": critical error occurred";
                    VLOG(1) << "    " << state.what();
                    break;
            }
        }); // Connection:keep-alive\r\n\r\n"
    }

    return core::SNodeC::start();
}
