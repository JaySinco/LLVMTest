#pragma once
#include "type.h"
#include <atomic>
#include <QThread>

class PacketsAcquirer: public QThread
{
    Q_OBJECT

public:
    explicit PacketsAcquirer(net::Ip4 hint, QObject* parent = nullptr);
    ~PacketsAcquirer() override;
    void run() override;
    void stop();

signals:
    void packetReceived(net::Packet const&);
    void stopped();

private:
    std::atomic<bool> should_stop_;
    net::Ip4 hint_;
};