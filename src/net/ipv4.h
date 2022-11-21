#pragma once
#include "protocol.h"

namespace net
{

class Ipv4: public Protocol
{
public:
    Ipv4() = default;
    Ipv4(Ip4 sip, Ip4 dip, uint8_t ttl, Type ipv4_type, bool forbid_slice);
    explicit Ipv4(BytesReader& reader);
    ~Ipv4() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    Type typeNext() const override;
    bool correlated(Protocol const& resp) const override;

    uint16_t headerSize() const;
    uint16_t payloadSize() const;
    void encodeHeader(BytesBuilder& builder) const;

    Ip4 sip() const { return sip_.v; };

    Ip4 dip() const { return dip_.v; };

private:
    DEFINE_PROP(uint8_t, ver_hl, "version (4 bits) + header length (4 bits)");
    DEFINE_PROP(uint8_t, tos, "type of service");
    mutable DEFINE_PROP(uint16_t, tlen, "total length");
    DEFINE_PROP(uint16_t, id, "identification");
    DEFINE_PROP(uint16_t, flags_fo, "flags (3 bits) + fragment offset (13 bits)");
    DEFINE_PROP(uint8_t, ttl, "time to live");
    DEFINE_PROP(uint8_t, ipv4_type, "ipv4 type");
    mutable DEFINE_PROP(uint16_t, crc, "header checksum");
    DEFINE_PROP(Ip4, sip, "source address");
    DEFINE_PROP(Ip4, dip, "destination address");
    DEFINE_PROP(std::vector<uint8_t>, opts, "variable length options");

    static constexpr size_t kFixedBytes = 20;
    uint16_t headerChecksum() const;
    static std::map<uint8_t, Type> table;
};

}  // namespace net