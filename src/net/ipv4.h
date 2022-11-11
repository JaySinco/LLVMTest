#pragma once
#include "protocol.h"

namespace net
{

class Ipv4: public Protocol
{
public:
    struct Header
    {
        uint8_t ver_hl;     // Version (4 bits) + Header length (4 bits)
        uint8_t tos;        // Type of service
        uint16_t tlen;      // Total length
        uint16_t id;        // Identification
        uint16_t flags_fo;  // Flags (3 bits) + Fragment offset (13 bits)
        uint8_t ttl;        // Time to live
        uint8_t type;       // IPv4 type
        uint16_t crc;       // Header checksum
        Ip4 sip;            // Source address
        Ip4 dip;            // Destination address
    };

    Ipv4() = default;
    Ipv4(Ip4 const& sip, Ip4 const& dip, uint8_t ttl, Type ipv4_type, bool forbid_slice);
    ~Ipv4() override = default;

    static void fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack);

    void toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;

    Header const& getHeader() const;
    Type ipv4Type() const;
    bool operator==(Ipv4 const& rhs) const;
    uint16_t payloadSize() const;

private:
    Header h_{0};
    static std::map<uint8_t, Type> type_dict;
    static Header ntoh(Header const& h, bool reverse = false);
    static Header hton(Header const& h);
};

}  // namespace net