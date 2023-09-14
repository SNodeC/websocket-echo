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
#include "web/http/client/Request.h"   // for Request, client
#include "web/http/client/Response.h"  // for Response
#include "web/http/legacy/in/Client.h" // for Client, Client<>...
#include "web/http/tls/in/Client.h"    // for Client, Client<>...

#include <bits/utility.h>
#include <cstdlib>
#include <openssl/asn1.h>    // for ASN1_STRING_get0_data, ASN1_STRING_length
#include <openssl/crypto.h>  // for OPENSSL_free
#include <openssl/obj_mac.h> // for NID_subject_alt_name
#include <openssl/ssl3.h>    // for SSL_get_peer_certificate, SSL_get_verify_result
#include <openssl/types.h>
#include <openssl/x509.h> // for X509_NAME_oneline, X509_free, X509_get_ext_d2i, X509_get_issuer_name, X509_get_subject_name, X509_verify_...
#include <openssl/x509v3.h>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

int main(int argc, char* argv[]) {
    core::SNodeC::init(argc, argv);

    setenv("DL_WSCLIENT_SUBPROTOCOL_PATH", CMAKE_CURRENT_BINARY_DIR "/subprotocol/client/echo", 0);

    {
        using LegacySocketAddress = web::http::legacy::in::Client<web::http::client::Request, web::http::client::Response>::SocketAddress;

        web::http::legacy::in::Client<web::http::client::Request, web::http::client::Response> legacyClient(
            "legacy",
            [](web::http::legacy::in::Client<web::http::client::Request, web::http::client::Response>::SocketConnection* socketConnection)
                -> void {
                VLOG(0) << "OnConnect";

                VLOG(0) << "\tServer: " + socketConnection->getRemoteAddress().toString();
                VLOG(0) << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            []([[maybe_unused]] web::http::legacy::in::Client<web::http::client::Request, web::http::client::Response>::SocketConnection*
                   socketConnection) -> void {
                VLOG(0) << "OnConnected";
            },
            [](web::http::client::Request& request) -> void {
                VLOG(0) << "OnRequestBegin";

                request.set("Sec-WebSocket-Protocol", "test, echo");

                request.upgrade("/ws/", "websocket");
            },
            [](web::http::client::Request& request, web::http::client::Response& response) -> void {
                VLOG(0) << "OnResponse";
                VLOG(0) << "     Status:";
                VLOG(0) << "       " << response.httpVersion << " " << response.statusCode << " " << response.reason;

                VLOG(0) << "     Headers:";
                for (const auto& [field, value] : response.headers) {
                    VLOG(0) << "       " << field + " = " + value;
                }

                VLOG(0) << "     Cookies:";
                for (const auto& [name, cookie] : response.cookies) {
                    VLOG(0) << "       " + name + " = " + cookie.getValue();
                    for (const auto& [option, value] : cookie.getOptions()) {
                        VLOG(0) << "         " + option + " = " + value;
                    }
                }

                response.body.push_back(0); // make it a c-string
                VLOG(0) << "Body:\n----------- start body -----------\n" << response.body.data() << "\n------------ end body ------------";

                response.upgrade(request);
            },
            [](int status, const std::string& reason) -> void {
                VLOG(0) << "OnResponseError";
                VLOG(0) << "     Status: " << status;
                VLOG(0) << "     Reason: " << reason;
            },
            [](web::http::legacy::in::Client<web::http::client::Request, web::http::client::Response>::SocketConnection* socketConnection)
                -> void {
                VLOG(0) << "OnDisconnect";

                VLOG(0) << "\tServer: " + socketConnection->getRemoteAddress().toString();
                VLOG(0) << "\tClient: " + socketConnection->getLocalAddress().toString();
            });
        legacyClient.connect([](const LegacySocketAddress& socketAddress, int errnum) -> void {
            if (errnum < 0) {
                PLOG(ERROR) << "OnError";
            } else if (errnum > 0) {
                PLOG(ERROR) << "OnError: " << socketAddress.toString();
            } else {
                VLOG(0) << "snode.c connecting to " << socketAddress.toString();
            }
        }); // Connection:keep-alive\r\n\r\n"

        std::map<std::string, std::any> options;
        options["CertChain"] = "/home/voc/projects/websocket-echo/certs/WebServerCertificateChain.pem";
        options["CertChainKey"] = "/home/voc/projects/websocket-echo/certs/Volker_Christian_-_WEB-Cert.pem";
        options["Password"] = "pentium5";
        options["CaFile"] = "/home/voc/projects/websocket-echo/certs/Client-Root-CA.crt";

        using TLSSocketAddress = web::http::tls::in::Client<web::http::client::Request, web::http::client::Response>::SocketAddress;

        web::http::tls::in::Client<web::http::client::Request, web::http::client::Response> tlsClient(
            "tls",
            [](web::http::tls::in::Client<web::http::client::Request, web::http::client::Response>::SocketConnection* socketConnection)
                -> void {
                VLOG(0) << "OnConnect";

                VLOG(0) << "\tServer: " + socketConnection->getRemoteAddress().toString();
                VLOG(0) << "\tClient: " + socketConnection->getLocalAddress().toString();
            },
            [](web::http::tls::in::Client<web::http::client::Request, web::http::client::Response>::SocketConnection* socketConnection)
                -> void {
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
            [](web::http::client::Request& request) -> void {
                VLOG(0) << "OnRequestBegin";

                request.set("Sec-WebSocket-Protocol", "test, echo");

                request.upgrade("/ws/", "websocket");
            },
            [](web::http::client::Request& request, web::http::client::Response& response) -> void {
                VLOG(0) << "OnResponse";
                VLOG(0) << "     Status:";
                VLOG(0) << "       " << response.httpVersion << " " << response.statusCode << " " << response.reason;

                VLOG(0) << "     Headers:";
                for (const auto& [field, value] : response.headers) {
                    VLOG(0) << "       " << field + " = " + value;
                }

                VLOG(0) << "     Cookies:";
                for (const auto& [name, cookie] : response.cookies) {
                    VLOG(0) << "       " + name + " = " + cookie.getValue();
                    for (const auto& [option, value] : cookie.getOptions()) {
                        VLOG(0) << "         " + option + " = " + value;
                    }
                }

                response.body.push_back(0); // make it a c-string
                VLOG(0) << "Body:\n----------- start body -----------\n" << response.body.data() << "\n------------ end body ------------";

                response.upgrade(request);
            },
            [](int status, const std::string& reason) -> void {
                VLOG(0) << "OnResponseError";
                VLOG(0) << "     Status: " << status;
                VLOG(0) << "     Reason: " << reason;
            },
            [](web::http::tls::in::Client<web::http::client::Request, web::http::client::Response>::SocketConnection* socketConnection)
                -> void {
                VLOG(0) << "OnDisconnect";

                VLOG(0) << "\tServer: " + socketConnection->getRemoteAddress().toString();
                VLOG(0) << "\tClient: " + socketConnection->getLocalAddress().toString();
            });

        tlsClient.connect("localhost", 8088, [](const TLSSocketAddress& socketAddress, int errnum) -> void {
            if (errnum < 0) {
                PLOG(ERROR) << "OnError";
            } else if (errnum > 0) {
                PLOG(ERROR) << "OnError: " << socketAddress.toString();
            } else {
                VLOG(0) << "snode.c connecting to " << socketAddress.toString();
            }
        }); // Connection:keep-alive\r\n\r\n"
    }

    return core::SNodeC::start();
}
