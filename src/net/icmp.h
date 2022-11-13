#pragma once
#include "protocol.h"
#include "ipv4.h"

namespace net
{

class Icmp: public Protocol
{
public:
    struct Header
    {
        uint8_t type;  // Type
        uint8_t code;  // Code
        uint16_t crc;  // Checksum as a whole
    };

    struct NetmaskHeader
    {                 // Type: 17,18
        uint16_t id;  // Identification
        uint16_t sn;  // Serial number
        Ip4 mask;     // Subnet mask
    };

    struct TimestampHeader
    {                   // Type: 13,14
        uint16_t id;    // Identification
        uint16_t sn;    // Serial number
        uint32_t init;  // Initiate timestamp
        uint32_t recv;  // Receive timestamp
        uint32_t send;  // Send timestamp
    };

    struct UnreachableHeader
    {                     // Type: 3
        uint32_t unused;  // Unused, must be 0
        Ipv4 eip;         // Error ip header
        uint8_t buf[8];   // At least 8 bytes behind ip header
    };

    struct PingHeader
    {                               // Type: 0,8
        uint16_t id;                // Identification
        uint16_t sn;                // Serial number
        std::vector<uint8_t> echo;  // Echo data
    };

    icmp() = default;

    icmp(u_char const* const start, u_char const*& end, protocol const* prev);

    icmp(std::string const& ping_echo);

    virtual ~icmp() = default;

    virtual void to_bytes(std::vector<u_char>& bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(protocol const& rhs) const override;

    detail const& get_detail() const;

    extra_detail const& get_extra() const;

    std::string icmp_type() const;

private:
    detail d{0};

    extra_detail extra;

    static std::map<std::pair<uint8_t, uint8_t>, std::string> type_dict;

    static detail ntoh(detail const& d, bool reverse = false);

    static detail hton(detail const& d);
};

}  // namespace net