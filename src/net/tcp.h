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
    bool correlated(Protocol const& resp) const override;

    Type tcpType() const;
    uint16_t headerSize() const;

private:
    uint16_t sport_;             // Source port
    uint16_t dport_;             // Destination port
    uint32_t sn_;                // Sequence number
    uint32_t an_;                // Acknowledgment number
    uint16_t hl_flags_;          // Header length (4 bits) + Reserved (3 bits) + Flags (9 bits)
    uint16_t wlen_;              // Window Size
    mutable uint16_t crc_;       // Checksum
    uint16_t urp_;               // Urgent pointer
    std::vector<uint8_t> opts_;  // Variable length options

    static constexpr size_t kFixedBytes = 20;
    void encodeHeader(BytesBuilder& builder) const;
    uint16_t overallChecksum(Ip4 sip, Ip4 dip, uint8_t const* payload, size_t size) const;
    static std::map<uint16_t, Type> table;
};

}  // namespace net