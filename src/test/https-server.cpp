#include "utils/base.h"
#include <uWebSockets/App.h>
#include <fmt/ostream.h>

int main(int argc, char** argv)
{
    auto key_file = (__DIRNAME__ / "key.pem").generic_string();
    auto cert_file = (__DIRNAME__ / "cert.pem").generic_string();

    uWS::SocketContextOptions opt;
    opt.key_file_name = key_file.c_str();
    opt.cert_file_name = cert_file.c_str();

    uWS::SSLApp(opt)
        .get("/*", [](auto* res, auto* req) { res->end("Hello world!"); })
        .listen(8080,
                [](auto* listen_socket) {
                    if (listen_socket) {
                        spdlog::info(
                            "listening on port {}",
                            us_socket_local_port(1, reinterpret_cast<us_socket_t*>(listen_socket)));
                    }
                })
        .run();
}
