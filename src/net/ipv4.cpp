#include "ipv4.h"
#include "platform.h"

namespace net
{

std::map<uint8_t, Protocol::Type> Ipv4::type_dict = {
    {1, Protocol::kICMP},
    {6, Protocol::kTCP},
    {17, Protocol::kUDP},
};

void Ipv4::fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack)
{
    auto p = std::make_shared<Ipv4>();
    p->h_ = ntoh(*reinterpret_cast<Header const*>(data));
    if (p->h_.tlen != size) {
        spdlog::warn("wrong ipv4 total length: expected={}, got={}", p->h_.tlen, size);
    }
    stack.push(p);
    size_t hs = 4 * (p->h_.ver_hl & 0xf);
    data += hs;
    size -= hs;

    switch (p->ipv4Type()) {
        case kICMP:
        case kTCP:
        case kUDP:
        default:
            break;
    }
}

Ipv4::Ipv4(Ip4 const& sip, Ip4 const& dip, uint8_t ttl, Type ipv4_type, bool forbid_slice)
{
    bool found = false;
    for (auto [k, v]: type_dict) {
        if (v == ipv4_type) {
            found = true;
            h_.type = k;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error(fmt::format("invalid ipv4 type: {}", descType(ipv4_type)));
    }
    h_.ver_hl = (4 << 4) | (sizeof(Header) / 4);
    h_.id = rand16u();
    h_.flags_fo = forbid_slice ? 0x4000 : 0;
    h_.ttl = ttl;
    h_.sip = sip;
    h_.dip = dip;
}

void Ipv4::toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    auto pt = const_cast<Ipv4*>(this);
    pt->h_.tlen = sizeof(Header) + bytes.size();

    auto hd = hton(h_);
    hd.crc = checksum(&hd, sizeof(Header));
    pt->h_.crc = hd.crc;

    auto it = reinterpret_cast<uint8_t const*>(&hd);
    bytes.insert(bytes.cbegin(), it, it + sizeof(h_));
}

Json Ipv4::toJson() const
{
    Json j;
    j["type"] = type();
    j["ipv4-type"] = descType(ipv4Type());
    j["version"] = h_.ver_hl >> 4;
    j["tos"] = h_.tos;
    size_t header_size = 4 * (h_.ver_hl & 0xf);
    j["header-size"] = header_size;
    int crc = -1;
    if (header_size == sizeof(Header)) {
        auto dt = hton(h_);
        crc = checksum(&dt, header_size);
    }
    j["header-checksum"] = crc;
    j["total-size"] = h_.tlen;
    j["id"] = h_.id;
    j["more-fragment"] = h_.flags_fo & 0x2000 ? true : false;
    j["forbid-slice"] = h_.flags_fo & 0x4000 ? true : false;
    j["fragment-offset"] = (h_.flags_fo & 0x1fff) * 8;
    j["ttl"] = static_cast<int>(h_.ttl);
    j["source-ip"] = h_.sip.toStr();
    j["dest-ip"] = h_.dip.toStr();
    return j;
}

Protocol::Type Ipv4::type() const { return kIPv4; }

bool Ipv4::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Ipv4 const&>(resp);
        return h_.sip == p.h_.dip;
    }
    return false;
}

Protocol::Type Ipv4::ipv4Type() const
{
    if (type_dict.count(h_.type) != 0) {
        return type_dict.at(h_.type);
    }
    return kUnknown;
}

Ipv4::Header const& Ipv4::getHeader() const { return h_; }

uint16_t Ipv4::payloadSize() const { return h_.tlen - 4 * (h_.ver_hl & 0xf); }

Ipv4::Header Ipv4::ntoh(Header const& h, bool reverse)
{
    Header hd = h;
    ntohx(hd.tlen, reverse, s);
    ntohx(hd.id, reverse, s);
    ntohx(hd.flags_fo, reverse, s);
    return hd;
}

Ipv4::Header Ipv4::hton(Header const& h) { return ntoh(h, true); }

bool Ipv4::operator==(Ipv4 const& rhs) const
{
    return h_.ver_hl == rhs.h_.ver_hl && h_.id == rhs.h_.id && h_.flags_fo == rhs.h_.flags_fo &&
           h_.type == rhs.h_.type && h_.sip == rhs.h_.sip && h_.dip == rhs.h_.dip;
}

}  // namespace net