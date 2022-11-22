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
    reader.read8u(ver_hl_);
    reader.read8u(tos_);
    reader.read16u(tlen_);
    reader.read16u(id_);
    reader.read16u(flags_fo_);
    reader.read8u(ttl_);
    reader.read8u(ipv4_type_);
    reader.read16u(crc_, false);
    reader.readIp4(sip_);
    reader.readIp4(dip_);
    reader.readBytes(opts_, headerSize() - kFixedBytes);
}

void Ipv4::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Ipv4>(reader);
    stack.push(p);

    switch (p->typeNext()) {
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
            ipv4_type_.v = k;
            break;
        }
    }
    if (!found) {
        MY_THROW("invalid ipv4 type: {}", typeDesc(ipv4_type));
    }

    ver_hl_.v = (4 << 4) | (kFixedBytes / 4);
    tos_.v = 0;
    tlen_.v = 0;
    id_.v = rand16u();
    flags_fo_.v = forbid_slice ? 0x4000 : 0;
    ttl_.v = ttl;
    crc_.v = 0;
    sip_.v = sip;
    dip_.v = dip;
}

void Ipv4::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    tlen_.v = headerSize() + bytes.size();
    crc_.v = headerChecksum();
    BytesBuilder builder;
    encodeHeader(builder);
    builder.prependTo(bytes);
}

Json Ipv4::toJson() const
{
    Json j;
    j["type"] = typeDesc(type());

    JSON_PROP(j, ver_hl_);
    j[ver_hl_t::k]["hint"] = FSTR("version:{};header-size:{};", ver_hl_.v >> 4, headerSize());
    JSON_PROP(j, tos_);
    JSON_PROP(j, tlen_);
    JSON_PROP(j, id_);
    JSON_PROP(j, flags_fo_);
    j[flags_fo_t::k]["hint"] =
        FSTR("{}{}fragment-offset:{};", flags_fo_.v & 0x2000 ? "more-fragment;" : "",
             flags_fo_.v & 0x4000 ? "forbid-slice;" : "", (flags_fo_.v & 0x1fff) * 8);
    JSON_PROP(j, ttl_);
    JSON_PROP(j, ipv4_type_);
    JSON_PROP(j, crc_);
    j[crc_t::k]["hint"] = FSTR("checked:{};", headerChecksum());
    JSON_PROP(j, sip_);
    JSON_PROP(j, dip_);
    JSON_PROP(j, opts_);
    return j;
}

Protocol::Type Ipv4::type() const { return kIPv4; }

bool Ipv4::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Ipv4 const&>(resp);
        return sip_.v == p.dip_.v;
    }
    return false;
}

Protocol::Type Ipv4::typeNext() const
{
    if (table.count(ipv4_type_.v) != 0) {
        return table.at(ipv4_type_.v);
    }
    return kUnknown;
}

void Ipv4::encodeHeader(BytesBuilder& builder) const
{
    builder.write8u(ver_hl_.v);
    builder.write8u(tos_.v);
    builder.write16u(tlen_.v);
    builder.write16u(id_.v);
    builder.write16u(flags_fo_.v);
    builder.write8u(ttl_.v);
    builder.write8u(ipv4_type_.v);
    builder.write16u(crc_.v, false);
    builder.writeIp4(sip_.v);
    builder.writeIp4(dip_.v);
    builder.writeBytes(opts_.v);
}

uint16_t Ipv4::headerChecksum() const
{
    BytesBuilder builder;
    encodeHeader(builder);
    return checksum(builder.data(), builder.size());
}

uint16_t Ipv4::headerSize() const { return 4 * (ver_hl_.v & 0xf); }

uint16_t Ipv4::payloadSize() const { return tlen_.v - headerSize(); }

}  // namespace net