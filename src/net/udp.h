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
    Type typeNext() const override;
    bool correlated(Protocol const& resp) const override;

private:
    DEFINE_PROP(uint16_t, sport, "source port");
    DEFINE_PROP(uint16_t, dport, "destination port");
    mutable DEFINE_PROP(uint16_t, tlen, "datagram length, >= 8");
    mutable DEFINE_PROP(uint16_t, crc, "checksum");

    static constexpr size_t kFixedBytes = 8;
    void encodeHeader(BytesBuilder& builder) const;
    uint16_t overallChecksum(Ip4 sip, Ip4 dip, uint8_t const* payload, size_t size) const;
    static std::map<uint16_t, Type> table;
};

}  // namespace net