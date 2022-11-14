#pragma once
#include "protocol.h"

namespace net
{

class Udp: public Protocol
{
public:
    Udp() = default;
    explicit Udp(BytesReader& reader);
    ~Udp() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;

    Type udpType() const;

private:
    uint16_t sport_;         // Source port
    uint16_t dport_;         // Destination port
    mutable uint16_t tlen_;  // Datagram length, >= 8
    mutable uint16_t crc_;   // Checksum

    static constexpr size_t kFixedBytes = 8;
    void encodeHeader(BytesBuilder& builder) const;
    uint16_t overallChecksum(Ip4 sip, Ip4 dip, uint8_t const* payload, size_t size) const;
    static std::map<uint16_t, Type> table;
};

}  // namespace net