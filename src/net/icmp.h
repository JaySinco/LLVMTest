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
    bool correlated(Protocol const& resp) const override;

private:
    uint8_t type_;          // Type
    uint8_t code_;          // Code
    mutable uint16_t crc_;  // Checksum as a whole
    uint16_t id_;           // Identification
    uint16_t sn_;           // Serial number
    std::vector<uint8_t> buf_;

    // Netmask: 17,18
    Ip4 mask_;  // Subnet mask

    // Timestamp: 13,14
    uint32_t init_;  // Initiate timestamp
    uint32_t recv_;  // Receive timestamp
    uint32_t send_;  // Send timestamp

    // Unreachable: 3
    Ipv4 eip_;  // Error ip header
    ;           // At least 8 bytes behind ip header

    // Ping: 0,8
    ;  // Echo data

    std::string icmpDesc() const;
    void encodeOverall(BytesBuilder& builder) const;
    uint16_t overallChecksum() const;
    static std::map<std::pair<uint8_t, uint8_t>, std::string> table;
};

}  // namespace net