#pragma once
#include "type.h"
#include <atomic>
#include <QThread>

class PacketsAcquirer: public QThread
{
    Q_OBJECT

public:
    explicit PacketsAcquirer(net::Ip4 hint, int bufmsec, QObject* parent);
    ~PacketsAcquirer() override;
    void run() override;
    void stop();

signals:
    void packetReceived(std::vector<net::Packet> const&);
    void stopped();

private:
    std::atomic<bool> should_stop_;
    net::Ip4 hint_;
    std::chrono::system_clock::time_point last_;
    const std::chrono::milliseconds bufmsec_;
    std::vector<net::Packet> buf_;
};