#pragma once
#include "protocol.h"

namespace net
{

class Arp: public Protocol
{
public:
    Arp() = default;
    Arp(Mac smac, Ip4 sip, Mac dmac, Ip4 dip, bool reply, bool reverse);
    explicit Arp(BytesReader& reader);
    ~Arp() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;

    Mac smac() const { return smac_; };

    Mac dmac() const { return dmac_; };

private:
    uint16_t hw_type_;    // Hardware type
    uint16_t prot_type_;  // Protocol type
    uint8_t hw_len_;      // Hardware address length
    uint8_t prot_len_;    // Protocol address length
    uint16_t op_;         // Operation code
    Mac smac_;            // Source ethernet address
    Ip4 sip_;             // Source ip address
    Mac dmac_;            // Destination ethernet address
    Ip4 dip_;             // Destination ip address
};

}  // namespace net