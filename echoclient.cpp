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
#include <SemanticLog.h>
#include "web/http/legacy/in/Client.h" // for Client, Client<>...
#include "web/http/tls/in/Client.h"    // for Client, Client<>...

#include <cstddef>
#include <openssl/ssl.h> // IWYU pragma: keep
#include <openssl/x509v3.h>
#include <string>

// IWYU pragma: no_include <openssl/asn1.h>
// IWYU pragma: no_include <openssl/crypto.h>
// IWYU pragma: no_include <openssl/obj_mac.h>
// IWYU pragma: no_include <openssl/ssl3.h>
// IWYU pragma: no_include <openssl/types.h>
// IWYU pragma: no_include <openssl/x509.h>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

int main(int argc, char* argv[]) {
    core::SNodeC::init(argc, argv);

    {
        using EchoClientLegacy = web::http::legacy::in::Client;
        using SocketConnectionLegacy = EchoClientLegacy::SocketConnection;
        using Request = EchoClientLegacy::Request;
        using Response = EchoClientLegacy::Response;
        using SocketAddressLegacy = EchoClientLegacy::SocketAddress;

        EchoClientLegacy legacyClient(
            "legacy",
            [](const SocketConnectionLegacy* socketConnection) -> void {
                snode::semantic::appLog().debug() << "OnConnect";

                snode::semantic::appLog().debug() << "\tServer: " + socketConnection->getRemoteAddress().toString();
                snode::semantic::appLog().debug() << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            [](const SocketConnectionLegacy* socketConnection) -> void {
                snode::semantic::appLog().debug() << "OnConnected";

                snode::semantic::appLog().debug() << "\tServer: " + socketConnection->getRemoteAddress().toString();
                snode::semantic::appLog().debug() << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            [](const SocketConnectionLegacy* socketConnection) -> void {
                snode::semantic::appLog().debug() << "OnDisconnect";

                snode::semantic::appLog().debug() << "\tServer: " + socketConnection->getRemoteAddress().toString();
                snode::semantic::appLog().debug() << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            [](const std::shared_ptr<Request>& request) -> void {
                snode::semantic::appLog().debug() << "OnRequestBegin";

                request->set("Sec-WebSocket-Protocol", "test, echo");

                request->upgrade(
                    "/ws/", "websocket", [](const std::shared_ptr<Request>& req, const std::shared_ptr<Response>& res) -> void {
                        snode::semantic::appLog().debug() << "OnResponse";
                        snode::semantic::appLog().debug() << "     Status:";
                        snode::semantic::appLog().debug() << "       " << res->httpVersion << " " << res->statusCode << " " << res->reason;

                        snode::semantic::appLog().debug() << "     Headers:";
                        for (const auto& [field, value] : res->headers) {
                            snode::semantic::appLog().debug() << "       " << field + " = " + value;
                        }

                        snode::semantic::appLog().debug() << "     Cookies:";
                        for (const auto& [name, cookie] : res->cookies) {
                            snode::semantic::appLog().debug() << "       " + name + " = " + cookie.getValue();
                            for (const auto& [option, value] : cookie.getOptions()) {
                                snode::semantic::appLog().debug() << "         " + option + " = " + value;
                            }
                        }

                        req->upgrade(res,
                                     [subProtocolsRequested = req->header("Upgrade"),
                                      &subProtocol = res->headers["upgrade"]](const std::string& name) -> void {
                                         if (!name.empty()) {
                                             snode::semantic::appLog().debug()
                                                 << "Successful upgrade to '" << subProtocol << "' requested: " << subProtocolsRequested;
                                         } else {
                                             snode::semantic::appLog().warn() << "Can not upgrade to '" << subProtocol << "' requested: " << subProtocolsRequested;
                                         }
                                     });
                    });
            },
            [](const std::shared_ptr<Request>& request) -> void {
                snode::semantic::appLog().debug() << "OnRequestEnd";
            });

        legacyClient.connect("localhost", 8080, [](const SocketAddressLegacy& socketAddress, const core::socket::State& state) -> void {
            switch (state) {
                case core::socket::State::OK:
                    snode::semantic::appLog().info() << "legacy: connected to '" << socketAddress.toString() << "': " << state.what();
                    break;
                case core::socket::State::DISABLED:
                    snode::semantic::appLog().info() << "legacy: disabled";
                    break;
                case core::socket::State::ERROR:
                    snode::semantic::appLog().warn() << "legacy: " << socketAddress.toString() << ": non critical error occurred";
                    snode::semantic::appLog().warn() << "    " << state.what();
                    break;
                case core::socket::State::FATAL:
                    snode::semantic::appLog().critical() << "legacy: " << socketAddress.toString() << ": critical error occurred";
                    snode::semantic::appLog().critical() << "    " << state.what();
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
            [](const SocketConnectionTLS* socketConnection) -> void {
                snode::semantic::appLog().debug() << "OnConnect";

                snode::semantic::appLog().debug() << "\tServer: " + socketConnection->getRemoteAddress().toString();
                snode::semantic::appLog().debug() << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            [](const SocketConnectionTLS* socketConnection) -> void {
                snode::semantic::appLog().debug() << "OnConnected";
                X509* server_cert = SSL_get_peer_certificate(socketConnection->getSSL());
                if (server_cert != nullptr) {
                    long verifyErr = SSL_get_verify_result(socketConnection->getSSL());

                    snode::semantic::appLog().debug() << "     Server certificate: " + std::string(X509_verify_cert_error_string(verifyErr));

                    char* str = X509_NAME_oneline(X509_get_subject_name(server_cert), nullptr, 0);
                    snode::semantic::appLog().debug() << "        Subject: " + std::string(str);
                    OPENSSL_free(str);

                    str = X509_NAME_oneline(X509_get_issuer_name(server_cert), nullptr, 0);
                    snode::semantic::appLog().debug() << "        Issuer: " + std::string(str);
                    OPENSSL_free(str);

                    // We could do all sorts of certificate verification stuff here
                    // before deallocating the certificate.

                    GENERAL_NAMES* subjectAltNames =
                        static_cast<GENERAL_NAMES*>(X509_get_ext_d2i(server_cert, NID_subject_alt_name, nullptr, nullptr));

                    int32_t altNameCount = sk_GENERAL_NAME_num(subjectAltNames);
                    snode::semantic::appLog().debug() << "        Subject alternative name count: " << altNameCount;
                    for (int32_t i = 0; i < altNameCount; ++i) {
                        GENERAL_NAME* generalName = sk_GENERAL_NAME_value(subjectAltNames, i);
                        if (generalName->type == GEN_URI) {
                            std::string subjectAltName =
                                std::string(reinterpret_cast<const char*>(ASN1_STRING_get0_data(generalName->d.uniformResourceIdentifier)),
                                            static_cast<std::size_t>(ASN1_STRING_length(generalName->d.uniformResourceIdentifier)));
                            snode::semantic::appLog().debug() << "           SAN (URI): '" + subjectAltName;
                        } else if (generalName->type == GEN_DNS) {
                            std::string subjectAltName =
                                std::string(reinterpret_cast<const char*>(ASN1_STRING_get0_data(generalName->d.dNSName)),
                                            static_cast<std::size_t>(ASN1_STRING_length(generalName->d.dNSName)));
                            snode::semantic::appLog().debug() << "           SAN (DNS): '" + subjectAltName;
                        } else {
                            snode::semantic::appLog().debug() << "           SAN (Type): '" + std::to_string(generalName->type);
                        }
                    }
                    sk_GENERAL_NAME_pop_free(subjectAltNames, GENERAL_NAME_free);

                    X509_free(server_cert);
                } else {
                    snode::semantic::appLog().debug() << "     Server certificate: no certificate";
                }
            },
            [](const SocketConnectionTLS* socketConnection) -> void {
                snode::semantic::appLog().debug() << "OnDisconnect";

                snode::semantic::appLog().debug() << "\tServer: " + socketConnection->getRemoteAddress().toString();
                snode::semantic::appLog().debug() << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            [](const std::shared_ptr<Request>& request) -> void {
                snode::semantic::appLog().debug() << "OnRequestBegin";

                request->set("Sec-WebSocket-Protocol", "test, echo");

                request->upgrade(
                    "/ws/", "websocket", [](const std::shared_ptr<Request>& req, const std::shared_ptr<Response>& res) -> void {
                        snode::semantic::appLog().debug() << "OnResponse";
                        snode::semantic::appLog().debug() << "     Status:";
                        snode::semantic::appLog().debug() << "       " << res->httpVersion << " " << res->statusCode << " " << res->reason;

                        snode::semantic::appLog().debug() << "     Headers:";
                        for (const auto& [field, value] : res->headers) {
                            snode::semantic::appLog().debug() << "       " << field + " = " + value;
                        }

                        snode::semantic::appLog().debug() << "     Cookies:";
                        for (const auto& [name, cookie] : res->cookies) {
                            snode::semantic::appLog().debug() << "       " + name + " = " + cookie.getValue();
                            for (const auto& [option, value] : cookie.getOptions()) {
                                snode::semantic::appLog().debug() << "         " + option + " = " + value;
                            }
                        }

                        req->upgrade(res,
                                     [subProtocolsRequested = req->header("Upgrade"),
                                      &subProtocol = res->headers["upgrade"]](const std::string& name) -> void {
                                         if (!name.empty()) {
                                             snode::semantic::appLog().debug()
                                                 << "Successful upgrade to '" << subProtocol << "' requested: " << subProtocolsRequested;
                                         } else {
                                             snode::semantic::appLog().warn() << "Can not upgrade to '" << subProtocol << "' requested: " << subProtocolsRequested;
                                         }
                                     });
                    });
            },
            [](const std::shared_ptr<Request>& request) -> void {
                snode::semantic::appLog().debug() << "OnRequestEnd";
            });

        tlsClient.connect("localhost", 8088, [](const SocketAddressTLS& socketAddress, const core::socket::State& state) -> void {
            switch (state) {
                case core::socket::State::OK:
                    snode::semantic::appLog().info() << "legacy: connected to '" << socketAddress.toString() << "': " << state.what();
                    break;
                case core::socket::State::DISABLED:
                    snode::semantic::appLog().info() << "legacy: disabled";
                    break;
                case core::socket::State::ERROR:
                    snode::semantic::appLog().warn() << "legacy: " << socketAddress.toString() << ": non critical error occurred";
                    snode::semantic::appLog().warn() << "    " << state.what();
                    break;
                case core::socket::State::FATAL:
                    snode::semantic::appLog().critical() << "legacy: " << socketAddress.toString() << ": critical error occurred";
                    snode::semantic::appLog().critical() << "    " << state.what();
                    break;
            }
        }); // Connection:keep-alive\r\n\r\n"
    }

    return core::SNodeC::start();
}
