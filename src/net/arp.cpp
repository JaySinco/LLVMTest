#include "arp.h"
#include "platform.h"

namespace net
{

Arp::Arp(BytesReader& reader)
{
    reader.read16u(hw_type_);
    reader.read16u(prot_type_);
    reader.read8u(hw_len_);
    reader.read8u(prot_len_);
    reader.read16u(op_);
    reader.readMac(smac_);
    reader.readIp4(sip_);
    reader.readMac(dmac_);
    reader.readIp4(dip_);
    reader.readAll(pad_);
}

void Arp::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Arp>(reader);
    stack.push(p);
}

Arp::Arp(Mac smac, Ip4 sip, Mac dmac, Ip4 dip, bool reply, bool reverse)
{
    hw_type_.v = 1;
    prot_type_.v = 0x0800;
    hw_len_.v = 6;
    prot_len_.v = 4;
    op_.v = reverse ? (reply ? 4 : 3) : (reply ? 2 : 1);
    smac_.v = smac;
    sip_.v = sip;
    dmac_.v = dmac;
    dip_.v = dip;
}

void Arp::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    BytesBuilder builder;
    builder.write16u(hw_type_.v);
    builder.write16u(prot_type_.v);
    builder.write8u(hw_len_.v);
    builder.write8u(prot_len_.v);
    builder.write16u(op_.v);
    builder.writeMac(smac_.v);
    builder.writeIp4(sip_.v);
    builder.writeMac(dmac_.v);
    builder.writeIp4(dip_.v);
    builder.prependTo(bytes);
}

Json Arp::toJson() const
{
    Json j;
    j["type"] = typeDesc(type());
    JSON_PROP(j, hw_type_);
    JSON_PROP(j, prot_type_);
    JSON_PROP(j, hw_len_);
    JSON_PROP(j, prot_len_);
    JSON_PROP(j, op_);
    j[op_t::k]["hint"] = (op_.v == 1 || op_.v == 3)   ? "request;"
                         : (op_.v == 2 || op_.v == 4) ? "reply;"
                                                      : FSTR("invalid(0x{:x});", op_.v);
    JSON_PROP(j, smac_);
    JSON_PROP(j, sip_);
    JSON_PROP(j, dmac_);
    JSON_PROP(j, dip_);
    return j;
}

Protocol::Type Arp::type() const
{
    return (op_.v == 1 || op_.v == 2) ? kARP : (op_.v == 3 || op_.v == 4) ? kRARP : kUnknown;
}

Protocol::Type Arp::typeNext() const { return kNull; }

bool Arp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Arp const&>(resp);
        return (op_.v == 1 || op_.v == 3) && (p.op_.v == 2 || p.op_.v == 4) && (dip_.v == p.sip_.v);
    }
    return false;
}

}  // namespace net