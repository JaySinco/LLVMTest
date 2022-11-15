#include "ipv4.h"
#include "platform.h"
#include "icmp.h"
#include "udp.h"
#include "tcp.h"

namespace net
{

std::map<uint8_t, Protocol::Type> Ipv4::table = {
    {1, Protocol::kICMP},
    {6, Protocol::kTCP},
    {17, Protocol::kUDP},
};

Ipv4::Ipv4(BytesReader& reader)
{
    ver_hl_ = reader.read8u();
    tos_ = reader.read8u();
    tlen_ = reader.read16u();
    id_ = reader.read16u();
    flags_fo_ = reader.read16u();
    ttl_ = reader.read8u();
    type_ = reader.read8u();
    crc_ = reader.read16u(false);
    sip_ = reader.readIp4();
    dip_ = reader.readIp4();
    opts_ = reader.readBytes(headerSize() - kFixedBytes);
}

void Ipv4::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Ipv4>(reader);
    stack.push(p);

    switch (p->ipv4Type()) {
        case kICMP:
            Icmp::decode(reader, stack);
            break;
        case kTCP:
            Tcp::decode(reader, stack);
            break;
        case kUDP:
            Udp::decode(reader, stack);
            break;
        default:
            Unimplemented::decode(reader, stack);
            break;
    }
}

Ipv4::Ipv4(Ip4 sip, Ip4 dip, uint8_t ttl, Type ipv4_type, bool forbid_slice)
{
    bool found = false;
    for (auto [k, v]: table) {
        if (v == ipv4_type) {
            found = true;
            type_ = k;
            break;
        }
    }
    if (!found) {
        MY_THROW("invalid ipv4 type: {}", descType(ipv4_type));
    }

    ver_hl_ = (4 << 4) | (kFixedBytes / 4);
    tos_ = 0;
    tlen_ = 0;
    id_ = rand16u();
    flags_fo_ = forbid_slice ? 0x4000 : 0;
    ttl_ = ttl;
    crc_ = 0;
    sip_ = sip;
    dip_ = dip;
}

void Ipv4::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    tlen_ = headerSize() + bytes.size();
    crc_ = headerChecksum();
    BytesBuilder builder;
    encodeHeader(builder);
    builder.prependTo(bytes);
}

Json Ipv4::toJson() const
{
    Json j;
    j["type"] = descType(type());
    Type ip_type = ipv4Type();
    j["ipv4-type"] = ip_type == kUnknown ? FSTR("unknown(0x{:x})", type_) : descType(ip_type);
    j["version"] = ver_hl_ >> 4;
    j["tos"] = tos_;
    j["header-size"] = headerSize();
    j["header-checksum"] = headerChecksum();
    j["total-size"] = tlen_;
    j["id"] = id_;
    j["more-fragment"] = flags_fo_ & 0x2000 ? true : false;
    j["forbid-slice"] = flags_fo_ & 0x4000 ? true : false;
    j["fragment-offset"] = (flags_fo_ & 0x1fff) * 8;
    j["ttl"] = static_cast<int>(ttl_);
    j["source-ip"] = sip_.toStr();
    j["dest-ip"] = dip_.toStr();
    return j;
}

Protocol::Type Ipv4::type() const { return kIPv4; }

bool Ipv4::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Ipv4 const&>(resp);
        return sip_ == p.dip_;
    }
    return false;
}

Protocol::Type Ipv4::ipv4Type() const
{
    if (table.count(type_) != 0) {
        return table.at(type_);
    }
    return kUnknown;
}

void Ipv4::encodeHeader(BytesBuilder& builder) const
{
    builder.write8u(ver_hl_);
    builder.write8u(tos_);
    builder.write16u(tlen_);
    builder.write16u(id_);
    builder.write16u(flags_fo_);
    builder.write8u(ttl_);
    builder.write8u(type_);
    builder.write16u(crc_, false);
    builder.writeIp4(sip_);
    builder.writeIp4(dip_);
    builder.writeBytes(opts_);
}

uint16_t Ipv4::headerChecksum() const
{
    BytesBuilder builder;
    encodeHeader(builder);
    return checksum(builder.data(), builder.size());
}

uint16_t Ipv4::headerSize() const { return 4 * (ver_hl_ & 0xf); }

uint16_t Ipv4::payloadSize() const { return tlen_ - headerSize(); }

}  // namespace net