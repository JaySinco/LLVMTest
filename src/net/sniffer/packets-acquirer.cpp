#include "packets-acquirer.h"
#include "driver.h"
#include <QMetaType>

PacketsAcquirer::PacketsAcquirer(net::Ip4 hint, QObject* parent): QThread(parent), hint_(hint)
{
    qRegisterMetaType<net::Packet>("net::Packet");
}

PacketsAcquirer::~PacketsAcquirer()
{
    stop();
    wait();
};

void PacketsAcquirer::run()
{
    should_stop_ = false;
    net::Driver driver(hint_);
    while (!should_stop_) {
        auto pac = driver.recv();
        if (!pac) {
            if (pac.error().typeof(net::Error::kPacketExpired)) {
                WLOG("skip expired packet");
            } else {
                ELOG(pac.error().what());
            }
            continue;
        }
        emit packetReceived(*pac);
    }
    DLOG("packets acquirer stopped!");
    emit stopped();
}

void PacketsAcquirer::stop() { should_stop_ = true; }