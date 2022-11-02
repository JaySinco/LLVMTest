#include "utils/base.h"
#include <uWebSockets/App.h>
#include <fmt/ostream.h>

int main(int argc, char** argv)
{
    auto keyFile = (__DIRNAME__ / "key.pem").generic_string();
    auto certFile = (__DIRNAME__ / "cert.pem").generic_string();

    uWS::SocketContextOptions opt;
    opt.key_file_name = keyFile.c_str();
    opt.cert_file_name = certFile.c_str();

    uWS::SSLApp(opt)
        .get("/*", [](auto* res, auto* /*req*/) { res->end("Hello world!"); })
        .listen(8080,
                [](auto* listen_socket) {
                    if (listen_socket) {
                        spdlog::info("listening on port {}",
                                     us_socket_local_port(1, (us_socket_t*)listen_socket));
                    }
                })
        .run();
}
