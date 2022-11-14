#pragma once
#include "protocol.h"

namespace net
{

class Transport
{
public:
    explicit Transport(Adaptor const& apt, int to_ms = 1000);
    ~Transport();

    ProtocolStack request(ProtocolStack const& req, int to_ms = -1, bool skip_send = false);
    Mac getMacFromIp4(Ip4 const& ip, bool use_cache = false, int to_ms = 5000);

    static ProtocolStack stackARP(Mac const& smac, Ip4 const& sip, Mac const& dmac, Ip4 const& dip,
                                  bool reply = false, bool reverse = false);

private:
    void send(Packet const& pac) const;
    utils::Expected<Packet> recv() const;

    Adaptor apt_;
    void* p_;
};

}  // namespace net