#include "../driver.h"
#include <signal.h>
#include <argparse/argparse.hpp>

std::atomic<bool> end_attack = false;

void onInterrupt(int) { end_attack = true; }

int main(int argc, char** argv)
{
    argparse::ArgumentParser prog("net-tools-arp");
    prog.add_argument("ip").required().help("ip4 address");
    prog.add_argument("-a", "--attack")
        .default_value(false)
        .implicit_value(true)
        .help("mobilize arp attack");

    try {
        prog.parse_args(argc, argv);
    } catch (std::exception const& err) {
        ELOG("{}\n", err.what());
        std::cerr << prog;
        std::exit(1);
    }

    MY_TRY;
    utils::initLogger(argv[0]);
    auto ip = net::Ip4::fromDottedDec(prog.get<std::string>("ip"));
    net::Driver driver(ip);
    if (!prog.get<bool>("--attack")) {
        auto mac = driver.getMac(ip);
        if (!mac) {
            if (mac.error().timeout()) {
                ILOG("{} is offline", ip.toStr());
            } else {
                ELOG(mac.error().what());
            }
        } else {
            ILOG("{} is at {}", ip.toStr(), (*mac).toStr());
        }

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
    MY_CATCH;
}
