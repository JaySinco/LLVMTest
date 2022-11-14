#include "transport.h"
#include <signal.h>
#include <argparse/argparse.hpp>

std::atomic<bool> end_attack = false;

void onInterrupt(int) { end_attack = true; }

int main(int argc, char** argv)
{
    argparse::ArgumentParser prog("net-tools-arp");
    prog.add_argument("ip").default_value("0.0.0.0").help("ipv4 address");
    prog.add_argument("-a", "--attack")
        .default_value(false)
        .implicit_value(true)
        .help("attack network by pretending myself to be gateway");

    try {
        prog.parse_args(argc, argv);
    } catch (std::exception const& err) {
        spdlog::error("{}\n", err.what());
        std::cerr << prog;
        std::exit(1);
    }

    TRY_;
    auto ip =
        prog.is_used("ip") ? net::Ip4::fromDottedDec(prog.get<std::string>("ip")) : net::Ip4::kNull;
    auto apt = net::Adaptor::fit(ip);
    net::Transport tr(apt);
    if (!prog.get<bool>("-attack")) {
        // mac mac_;
        // if (transport::ip2mac(handle, ip, mac_)) {
        //     std::cout << ip.to_str() << " is at " << mac_.to_str() << "." << std::endl;
        // } else {
        //     std::cout << ip.to_str() << " is offline." << std::endl;
        // }
    } else {
        signal(SIGINT, onInterrupt);
        // mac gateway_mac;
        // if (transport::ip2mac(handle, apt.gateway, gateway_mac)) {
        //     LOG(INFO) << "gateway {} is at {}"_format(apt.gateway.to_str(),
        //     gateway_mac.to_str());
        // }
        // LOG(INFO) << "forging gateway's mac to {}..."_format(apt.mac_.to_str());
        // auto lie = packet::arp(apt.mac_, apt.gateway, apt.mac_, apt.ip, true);
        // while (!end_attack) {
        //     transport::send(handle, lie);
        //     std::this_thread::sleep_for(1000ms);
        // }
        // LOG(INFO) << "attack stopped";
        // if (transport::ip2mac(handle, apt.gateway, gateway_mac, false)) {
        //     auto truth = packet::arp(gateway_mac, apt.gateway, apt.mac_, apt.ip, true);
        //     transport::send(handle, truth);
        //     LOG(INFO) << "gateway's mac restored to {}"_format(gateway_mac.to_str());
        // }
    }
    CATCH_;
}
