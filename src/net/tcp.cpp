#include "tcp.h"
#include "ipv4.h"
#include <boost/algorithm/string/join.hpp>

namespace net
{

std::map<uint16_t, Protocol::Type> Tcp::table = {
    {22, kSSH}, {23, kTelnet}, {53, kDNS}, {80, kHTTP}, {443, kHTTPS}, {3389, kRDP},
};

Tcp::Tcp(BytesReader& reader)
{
    reader.read16u(sport_);
    reader.read16u(dport_);
    reader.read32u(sn_);
    reader.read32u(an_);
    reader.read16u(hl_flags_);
    reader.read16u(wlen_);
    reader.read16u(crc_, false);
    reader.read16u(urp_);
    reader.readBytes(opts_, headerSize() - kFixedBytes);
}

void Tcp::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Tcp>(reader);
    stack.push(p);

    switch (p->typeNext()) {
        default:
            Unimplemented::decode(reader, stack);
            break;
    }
}

void Tcp::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    auto& ipv4 = stack.get<Ipv4>(kIPv4);
    crc_.v = overallChecksum(ipv4.sip(), ipv4.dip(), bytes.data(), bytes.size());
    BytesBuilder builder;
    encodeHeader(builder);
    builder.prependTo(bytes);
}

Json Tcp::toJson() const
{
    Json j;
    j["type"] = typeDesc(type());
    JSON_PROP(j, sport_);
    JSON_PROP(j, dport_);
    JSON_PROP(j, sn_);
    JSON_PROP(j, an_);
    JSON_PROP(j, hl_flags_);

    std::vector<std::string> flags;
    if (hl_flags_.v >> 8 & 0x1) {
        flags.push_back("ns");
    }
    if (hl_flags_.v >> 7 & 0x1) {
        flags.push_back("cwr");
    }
    if (hl_flags_.v >> 6 & 0x1) {
        flags.push_back("ece");
    }
    if (hl_flags_.v >> 5 & 0x1) {
        flags.push_back("urg");
    }
    if (hl_flags_.v >> 4 & 0x1) {
        flags.push_back("ack");
    }
    if (hl_flags_.v >> 3 & 0x1) {
        flags.push_back("psh");
    }
    if (hl_flags_.v >> 2 & 0x1) {
        flags.push_back("rst");
    }
    if (hl_flags_.v >> 1 & 0x1) {
        flags.push_back("syn");
    }
    if (hl_flags_.v & 0x1) {
        flags.push_back("fin");
    }
    j[hl_flags_t::k]["hint"] =
        FSTR("header-size:{};{};", headerSize(), boost::algorithm::join(flags, ";"));

    JSON_PROP(j, wlen_);
    JSON_PROP(j, crc_);
    JSON_PROP(j, urp_);
    JSON_PROP(j, opts_);
    return j;
}

Protocol::Type Tcp::type() const { return kTCP; }

Protocol::Type Tcp::typeNext() const
{
    if (table.count(dport_.v) != 0) {
        return table.at(dport_.v);
    }
    if (table.count(sport_.v) != 0) {
        return table.at(sport_.v);
    }
    return kUnknown;
}

bool Tcp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Tcp const&>(resp);
        return sport_.v == p.dport_.v && dport_.v == p.sport_.v;
    }
    return false;
}

uint16_t Tcp::headerSize() const { return 4 * (hl_flags_.v >> 12 & 0xf); }

void Tcp::encodeHeader(BytesBuilder& builder) const
{
    builder.write16u(sport_.v);
    builder.write16u(dport_.v);
    builder.write32u(sn_.v);
    builder.write32u(an_.v);
    builder.write16u(hl_flags_.v);
    builder.write16u(wlen_.v);
    builder.write16u(crc_.v, false);
    builder.write16u(urp_.v);
    builder.writeBytes(opts_.v);
}

uint16_t Tcp::overallChecksum(Ip4 sip, Ip4 dip, uint8_t const* payload, size_t size) const
{
    BytesBuilder builder;
    builder.writeIp4(sip);  // pseudo header
    builder.writeIp4(dip);
    builder.write8u(0);
    builder.write8u(6);
    builder.write16u(headerSize() + size);
    encodeHeader(builder);              // udp header
    builder.writeBytes(payload, size);  // data
    return checksum(builder.data(), builder.size());
}

}  // namespace net