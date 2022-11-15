#include "arp.h"
#include "platform.h"

namespace net
{

Arp::Arp(BytesReader& reader)
{
    hw_type_ = reader.read16u();
    prot_type_ = reader.read16u();
    hw_len_ = reader.read8u();
    prot_len_ = reader.read8u();
    op_ = reader.read16u();
    smac_ = reader.readMac();
    sip_ = reader.readIp4();
    dmac_ = reader.readMac();
    dip_ = reader.readIp4();
    reader.readAll();  // consume ethernet padding
}

void Arp::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Arp>(reader);
    stack.push(p);
}

Arp::Arp(Mac smac, Ip4 sip, Mac dmac, Ip4 dip, bool reply, bool reverse)
{
    hw_type_ = 1;
    prot_type_ = 0x0800;
    hw_len_ = 6;
    prot_len_ = 4;
    op_ = reverse ? (reply ? 4 : 3) : (reply ? 2 : 1);
    smac_ = smac;
    sip_ = sip;
    dmac_ = dmac;
    dip_ = dip;
}

void Arp::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    BytesBuilder builder;
    builder.write16u(hw_type_);
    builder.write16u(prot_type_);
    builder.write8u(hw_len_);
    builder.write8u(prot_len_);
    builder.write16u(op_);
    builder.writeMac(smac_);
    builder.writeIp4(sip_);
    builder.writeMac(dmac_);
    builder.writeIp4(dip_);
    builder.prependTo(bytes);
}

Json Arp::toJson() const
{
    Json j;
    j["type"] = descType(type());
    j["hardware-type"] = hw_type_;
    j["protocol-type"] = prot_type_;
    j["hardware-addr-len"] = hw_len_;
    j["protocol-addr-len"] = prot_len_;
    j["operate"] = (op_ == 1 || op_ == 3)   ? "request"
                   : (op_ == 2 || op_ == 4) ? "reply"
                                            : FSTR("invalid(0x{:x})", op_);
    j["source-mac"] = smac_.toStr();
    j["source-ip"] = sip_.toStr();
    j["dest-mac"] = dmac_.toStr();
    j["dest-ip"] = dip_.toStr();
    return j;
}

Protocol::Type Arp::type() const
{
    return (op_ == 1 || op_ == 2) ? kARP : (op_ == 3 || op_ == 4) ? kRARP : kUnknown;
}

bool Arp::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Arp const&>(resp);
        return (op_ == 1 || op_ == 3) && (p.op_ == 2 || p.op_ == 4) && (dip_ == p.sip_);
    }
    return false;
}

}  // namespace net