#pragma once
#include "protocol.h"
#include "ipv4.h"

namespace net
{

class Icmp: public Protocol
{
public:
    Icmp() = default;
    explicit Icmp(BytesReader& reader);
    static Icmp pingAsk(std::string const& echo);
    ~Icmp() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    Type typeNext() const override;
    bool correlated(Protocol const& resp) const override;

private:
    DEFINE_PROP(uint8_t, icmp_type, "icmp type");
    DEFINE_PROP(uint8_t, code, "icmp code");
    mutable DEFINE_PROP(uint16_t, crc, "checksum as a whole");
    DEFINE_PROP(uint16_t, id, "identification");
    DEFINE_PROP(uint16_t, sn, "serial number");

    // Netmask: 17,18
    DEFINE_PROP(Ip4, mask, "subnet mask");

    // Timestamp: 13,14
    DEFINE_PROP(uint32_t, init, "initiate timestamp");
    DEFINE_PROP(uint32_t, recv, "receive timestamp");
    DEFINE_PROP(uint32_t, send, "send timestamp");

    // Unreachable: 3
    DEFINE_PROP(uint32_t, unused, "unused");
    Ipv4 eip_;  // error ip header
    DEFINE_PROP(std::vector<uint8_t>, aft_ip, "at least 8 bytes behind ip header");

    // Ping: 0,8
    DEFINE_PROP(std::string, echo, "echo data");

    std::string icmpDesc() const;
    void encodeOverall(BytesBuilder& builder) const;
    uint16_t overallChecksum() const;
    static std::map<std::pair<uint8_t, uint8_t>, std::string> table;
};

}  // namespace net