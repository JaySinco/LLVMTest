#pragma once
#include "protocol.h"

namespace net
{

class Arp: public Protocol
{
public:
    struct Header
    {
        uint16_t hw_type;    // Hardware type
        uint16_t prot_type;  // Protocol type
        uint8_t hw_len;      // Hardware address length
        uint8_t prot_len;    // Protocol address length
        uint16_t op;         // Operation code
        Mac smac;            // Source ethernet address
        Ip4 sip;             // Source ip address
        Mac dmac;            // Destination ethernet address
        Ip4 dip;             // Destination ip address
    };

    Arp() = default;
    Arp(Mac const& smac, Ip4 const& sip, Mac const& dmac, Ip4 const& dip, bool reply, bool reverse);
    ~Arp() override = default;

    static void fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack);

    void toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;

    Header const& getHeader() const;

private:
    Header h_{0};
    static Header ntoh(Header const& h, bool reverse = false);
    static Header hton(Header const& h);
};

}  // namespace net