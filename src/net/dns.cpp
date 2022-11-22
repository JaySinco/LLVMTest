#include "dns.h"
#include <boost/algorithm/string.hpp>

namespace net
{

Dns::Dns(BytesReader& reader)
{
    reader.read16u(id_);
    reader.read16u(flags_);
    reader.read16u(qrn_);
    reader.read16u(rrn_);
    reader.read16u(arn_);
    reader.read16u(ern_);

    for (int i = 0; i < qrn_.v; ++i) {
        query_.push_back(decodeQuery(reader));
    }
    for (int i = 0; i < rrn_.v; ++i) {
        reply_.push_back(decodeResource(reader));
    }
    for (int i = 0; i < arn_.v; ++i) {
        auth_.push_back(decodeResource(reader));
    }
    for (int i = 0; i < ern_.v; ++i) {
        extra_.push_back(decodeResource(reader));
    }
}

Dns Dns::query(std::string const& domain)
{
    Dns p;
    p.id_.v = rand16u();
    p.flags_.v |= 0 << 15;          // qr
    p.flags_.v |= (0 & 0xf) << 11;  // opcode
    p.flags_.v |= 0 << 10;          // authoritative answer
    p.flags_.v |= 1 << 9;           // truncated
    p.flags_.v |= 1 << 8;           // recursion desired
    p.flags_.v |= 0 << 7;           // recursion available
    p.flags_.v |= (0 & 0xf);        // rcode
    p.qrn_.v = 1;
    p.rrn_.v = 0;
    p.arn_.v = 0;
    p.ern_.v = 0;

    Query qr;
    qr.domain_.v = domain;
    qr.dm_type_.v = 1;  // A
    qr.cls_.v = 1;      // internet address
    p.query_.push_back(qr);

    return p;
}

void Dns::decode(BytesReader& reader, ProtocolStack& stack)
{
    auto p = std::make_shared<Dns>(reader);
    stack.push(p);
}

void Dns::encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const
{
    BytesBuilder builder;
    builder.write16u(id_.v);
    builder.write16u(flags_.v);
    builder.write16u(qrn_.v);
    builder.write16u(rrn_.v);
    builder.write16u(arn_.v);
    builder.write16u(ern_.v);

    for (auto const& q: query_) {
        encodeQuery(builder, q);
    }
    for (auto const& r: reply_) {
        encodeResource(builder, r);
    }
    for (auto const& a: auth_) {
        encodeResource(builder, a);
    }
    for (auto const& e: extra_) {
        encodeResource(builder, e);
    }

    builder.prependTo(bytes);
}

Json Dns::toJson() const
{
    Json j;
    j["type"] = typeDesc(type());
    JSON_PROP(j, id_);
    JSON_PROP(j, flags_);

    std::vector<std::string> flags;
    if (flags_.v & 0x400) {
        flags.push_back("authoritative-answer");
    }
    if (flags_.v & 0x200) {
        flags.push_back("truncated");
    }
    if (flags_.v & 0x100) {
        flags.push_back("recursion-desired");
    }
    if (flags_.v & 0x80) {
        flags.push_back("recursion-available");
    }
    j[flags_t::k]["hint"] =
        FSTR("{};op:{};code:{};{};", flags_.v & 0x8000 ? "reply" : "query", (flags_.v >> 11) & 0xf,
             flags_.v & 0xf, boost::algorithm::join(flags, ";"));

    JSON_PROP(j, qrn_);
    JSON_PROP(j, rrn_);
    JSON_PROP(j, arn_);
    JSON_PROP(j, ern_);

    if (qrn_.v > 0) {
        Json::array_t query;
        for (auto const& q: query_) {
            query.push_back(toJsonQuery(q));
        }
        j["query"] = query;
    }

    if (rrn_.v > 0) {
        Json::array_t reply;
        for (auto const& r: reply_) {
            reply.push_back(toJsonResource(r));
        }
        j["reply"] = reply;
    }

    if (arn_.v > 0) {
        Json::array_t auth;
        for (auto const& a: auth_) {
            auth.push_back(toJsonResource(a));
        }
        j["auth"] = auth;
    }

    if (ern_.v > 0) {
        Json::array_t extra;
        for (auto const& e: extra_) {
            extra.push_back(toJsonResource(e));
        }
        j["auth"] = extra;
    }

    return j;
}

Json Dns::toJsonQuery(Query const& query)
{
    Json q;
    q["type"] = "dns-query";
    JSON_PROP(q, query.domain_);
    JSON_PROP(q, query.dm_type_);
    JSON_PROP(q, query.cls_);
    return q;
}

Json Dns::toJsonResource(Resource const& res)
{
    Json r;
    r["type"] = "dns-resource";
    JSON_PROP(r, res.domain_);
    JSON_PROP(r, res.dm_type_);
    JSON_PROP(r, res.cls_);
    JSON_PROP(r, res.ttl_);
    JSON_PROP(r, res.data_);
    if (res.dm_type_.v == 1) {  // A
        r[Resource::data_t::k]["hint"] = reinterpret_cast<Ip4 const*>(res.data_.v.data())->toStr();
    } else if (res.dm_type_.v == 1) {  // CNAME
        r[Resource::data_t::k]["hint"] = std::string(res.data_.v.begin(), res.data_.v.end());
    }
    return r;
}

Protocol::Type Dns::type() const { return kDNS; }

Protocol::Type Dns::typeNext() const { return kNull; }

bool Dns::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Dns const&>(resp);
        return id_.v == p.id_.v;
    }
    return false;
}

void Dns::encodeQuery(BytesBuilder& builder, Query const& query)
{
    builder.writeDomain(query.domain_.v);
    builder.write16u(query.dm_type_.v);
    builder.write16u(query.cls_.v);
}

Dns::Query Dns::decodeQuery(BytesReader& reader)
{
    Query query;
    reader.readDomain(query.domain_);
    reader.read16u(query.dm_type_);
    reader.read16u(query.cls_);
    return query;
}

void Dns::encodeResource(BytesBuilder& builder, Resource const& res)
{
    builder.writeDomain(res.domain_.v);
    builder.write16u(res.dm_type_.v);
    builder.write16u(res.cls_.v);
    builder.write32u(res.ttl_.v);
    builder.write16u(res.dlen_.v);
    builder.writeBytes(res.data_.v);
}

Dns::Resource Dns::decodeResource(BytesReader& reader)
{
    Resource res;
    reader.readDomain(res.domain_);
    reader.read16u(res.dm_type_);
    reader.read16u(res.cls_);
    reader.read32u(res.ttl_);
    reader.read16u(res.dlen_);
    reader.readBytes(res.data_, res.dlen_.v);
    return res;
}

}  // namespace net