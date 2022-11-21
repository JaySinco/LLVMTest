#pragma once
#include "protocol.h"

namespace net
{

class Tcp: public Protocol
{
public:
    Tcp() = default;
    explicit Tcp(BytesReader& reader);
    ~Tcp() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    Type typeNext() const override;
    bool correlated(Protocol const& resp) const override;

    uint16_t headerSize() const;

private:
    DEFINE_PROP(uint16_t, sport, "source port");
    DEFINE_PROP(uint16_t, dport, "destination port");
    DEFINE_PROP(uint32_t, sn, "sequence number");
    DEFINE_PROP(uint32_t, an, "acknowledgment number");
    DEFINE_PROP(uint16_t, hl_flags, "header length (4 bits) + reserved (3 bits) + flags (9 bits)");
    DEFINE_PROP(uint16_t, wlen, "window size");
    mutable DEFINE_PROP(uint16_t, crc, "checksum");
    DEFINE_PROP(uint16_t, urp, "urgent pointer");
    DEFINE_PROP(std::vector<uint8_t>, opts, "variable length options");

    static constexpr size_t kFixedBytes = 20;
    void encodeHeader(BytesBuilder& builder) const;
    uint16_t overallChecksum(Ip4 sip, Ip4 dip, uint8_t const* payload, size_t size) const;
    static std::map<uint16_t, Type> table;
};

}  // namespace net