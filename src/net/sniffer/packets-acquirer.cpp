#include "packets-acquirer.h"
#include "driver.h"
#include <QMetaType>

Q_DECLARE_METATYPE(net::Packet);

PacketsAcquirer::PacketsAcquirer(net::Ip4 hint, int bufmsec, QObject* parent)
    : QThread(parent), hint_(hint), last_(std::chrono::system_clock::now()), bufmsec_(bufmsec)
{
    qRegisterMetaType<std::vector<net::Packet>>("vector<Packet>");
}

PacketsAcquirer::~PacketsAcquirer()
{
    stop();
    wait();
};

void PacketsAcquirer::run()
{
    MY_TRY;
    should_stop_ = false;
    net::Driver driver(hint_);
    while (!should_stop_) {
        auto pac = driver.recv();
        if (!pac) {
            if (pac.error().typeof(net::Error::kPacketUnavailable)) {
                // pass
            } else {
                ELOG(pac.error().what());
            }
            continue;
        }
        buf_.push_back(std::move(*pac));
        auto now = std::chrono::system_clock::now();
        if (now - last_ > bufmsec_) {
            emit packetReceived(buf_);
            buf_.clear();
            last_ = now;
        }
    }
    ILOG("packets acquirer stopped!");
    emit stopped();
    MY_CATCH;
}

void PacketsAcquirer::stop() { should_stop_ = true; }