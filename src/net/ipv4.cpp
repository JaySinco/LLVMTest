#include "ipv4.h"
#include "platform.h"

namespace net
{

std::map<uint8_t, Protocol::Type> Ipv4::type_dict = {
    {1, Protocol::kICMP},
    {6, Protocol::kTCP},
    {17, Protocol::kUDP},
};

Ipv4::Ipv4(uint8_t const*& data, size_t& size)
{
    if (size < sizeof(Header)) {
        THROW_("inadequate bytes for ipv4 header");
    }
    Header hd = ntoh(*reinterpret_cast<Header const*>(data));
    if (hd.tlen != size) {
        THROW_(fmt::format("invalid ipv4 total length: {} expected, got {}", hd.tlen, size));
    }
    size_t hs = 4 * (hd.ver_hl & 0xf);
    uint8_t* buf = new uint8_t[hs]{0};
    std::memcpy(buf, &hd, sizeof(Header));
    std::memcpy(buf + sizeof(Header), data + sizeof(Header), hs - sizeof(Header));
    h_ = reinterpret_cast<Header*>(buf);
    data += hs;
    size -= hs;
}

Ipv4::~Ipv4() { delete[] reinterpret_cast<uint8_t*>(h_); }

void Ipv4::fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack)
{
    auto p = std::make_shared<Ipv4>(data, size);
    stack.push(p);
    switch (p->ipv4Type()) {
        case kICMP:
        case kTCP:
        case kUDP:
        default:
            Unimplemented::fromBytes(data, size, stack);
            break;
    }
}

Ipv4::Ipv4(Ip4 const& sip, Ip4 const& dip, uint8_t ttl, Type ipv4_type, bool forbid_slice)
{
    bool found = false;
    for (auto [k, v]: type_dict) {
        if (v == ipv4_type) {
            found = true;
            h_->type = k;
            break;
        }
    }
    if (!found) {
        THROW_(fmt::format("invalid ipv4 type: {}", descType(ipv4_type)));
    }
    uint8_t* buf = new uint8_t[sizeof(Header)]{0};
    h_ = reinterpret_cast<Header*>(buf);
    h_->id = rand16u();
    h_->ver_hl = (4 << 4) | (sizeof(Header) / 4);
    h_->flags_fo = forbid_slice ? 0x4000 : 0;
    h_->ttl = ttl;
    h_->sip = sip;
    h_->dip = dip;
}

void Ipv4::toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    auto pt = const_cast<Ipv4*>(this);
    size_t hs = headerSize();
    pt->h_->tlen = hs + bytes.size();

    auto hd = hton(h_);
    std::vector<uint8_t> buf;
    buf.insert(buf.cbegin(), opt_.begin(), opt_.end());
    auto it = reinterpret_cast<uint8_t const*>(&hd);
    buf.insert(buf.cbegin(), it, it + sizeof(h_));
    hd.crc = checksum(buf.data(), buf.size());
    pt->h_.crc = hd.crc;

    bytes.insert(bytes.cbegin(), opt_.begin(), opt_.end());
    bytes.insert(bytes.cbegin(), it, it + sizeof(h_));
}

Json Ipv4::toJson() const
{
    Json j;
    j["type"] = descType(type());
    Type ip_type = ipv4Type();
    j["ipv4-type"] =
        ip_type == kUnknown ? fmt::format("unknown(0x{:x})", h_.type) : descType(ip_type);
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

uint16_t Ipv4::headerSize() const { return 4 * (h_->ver_hl & 0xf); }

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