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
    Type typeNext() const override;
    bool correlated(Protocol const& resp) const override;

    Mac smac() const { return smac_.v; };

    Mac dmac() const { return dmac_.v; };

    Ip4 sip() const { return sip_.v; };

    Ip4 dip() const { return dip_.v; };

private:
    DEFINE_PROP(uint16_t, hw_type, "hardware type");
    DEFINE_PROP(uint16_t, prot_type, "protocol type");
    DEFINE_PROP(uint8_t, hw_len, "hardware address length");
    DEFINE_PROP(uint8_t, prot_len, "protocol address length");
    DEFINE_PROP(uint16_t, op, "operation code");
    DEFINE_PROP(Mac, smac, "source ethernet address");
    DEFINE_PROP(Ip4, sip, "source ip address");
    DEFINE_PROP(Mac, dmac, "destination ethernet address");
    DEFINE_PROP(Ip4, dip, "destination ip address");
    DEFINE_PROP(std::vector<uint8_t>, pad, "ethernet padding");
};

}  // namespace net