#pragma once
#include "protocol.h"

namespace net
{

class Ipv4: public Protocol
{
public:
    Ipv4() = default;
    Ipv4(Ip4 const& sip, Ip4 const& dip, uint8_t ttl, Type ipv4_type, bool forbid_slice);
    explicit Ipv4(BytesReader& reader);
    ~Ipv4() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;

    Type ipv4Type() const;
    uint16_t headerSize() const;
    uint16_t payloadSize() const;
    void encodeHeader(BytesBuilder& builder) const;

    Ip4 sip() const { return sip_; };

    Ip4 dip() const { return dip_; };

private:
    uint8_t ver_hl_;             // Version (4 bits) + Header length (4 bits)
    uint8_t tos_;                // Type of service
    mutable uint16_t tlen_;      // Total length
    uint16_t id_;                // Identification
    uint16_t flags_fo_;          // Flags (3 bits) + Fragment offset (13 bits)
    uint8_t ttl_;                // Time to live
    uint8_t type_;               // IPv4 type
    mutable uint16_t crc_;       // Header checksum
    Ip4 sip_;                    // Source address
    Ip4 dip_;                    // Destination address
    std::vector<uint8_t> opts_;  // Variable length options

    uint16_t headerChecksum() const;
    static constexpr size_t kFixedBytes = 20;
    static std::map<uint8_t, Type> table;
};

}  // namespace net