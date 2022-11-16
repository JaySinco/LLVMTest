#include "dns.h"
#include <boost/algorithm/string.hpp>

namespace net
{

Dns::Dns(BytesReader& reader)
{
    id_ = reader.read16u();
    flags_ = reader.read16u();
    qrn_ = reader.read16u();
    rrn_ = reader.read16u();
    arn_ = reader.read16u();
    ern_ = reader.read16u();

    for (int i = 0; i < qrn_; ++i) {
        query_.push_back(decodeQuery(reader));
    }
    for (int i = 0; i < rrn_; ++i) {
        reply_.push_back(decodeResource(reader));
    }
    for (int i = 0; i < arn_; ++i) {
        auth_.push_back(decodeResource(reader));
    }
    for (int i = 0; i < ern_; ++i) {
        extra_.push_back(decodeResource(reader));
    }
}

Dns Dns::query(std::string const& domain)
{
    Dns p;
    p.id_ = rand16u();
    p.flags_ |= 0 << 15;          // qr
    p.flags_ |= (0 & 0xf) << 11;  // opcode
    p.flags_ |= 0 << 10;          // authoritative answer
    p.flags_ |= 1 << 9;           // truncated
    p.flags_ |= 1 << 8;           // recursion desired
    p.flags_ |= 0 << 7;           // recursion available
    p.flags_ |= (0 & 0xf);        // rcode
    p.qrn_ = 1;
    p.rrn_ = 0;
    p.arn_ = 0;
    p.ern_ = 0;

    Query qr;
    qr.domain = domain;
    qr.type = 1;  // A
    qr.cls = 1;   // internet address
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
    builder.write16u(id_);
    builder.write16u(flags_);
    builder.write16u(qrn_);
    builder.write16u(rrn_);
    builder.write16u(arn_);
    builder.write16u(ern_);

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
    j["type"] = descType(type());
    j["id"] = id_;
    j["dns-type"] = flags_ & 0x8000 ? "reply" : "query";
    j["opcode"] = (flags_ >> 11) & 0xf;
    j["authoritative-answer"] = flags_ & 0x400 ? true : false;
    j["truncated"] = flags_ & 0x200 ? true : false;
    j["recursion-desired"] = flags_ & 0x100 ? true : false;
    j["recursion-available"] = flags_ & 0x80 ? true : false;
    j["rcode"] = flags_ & 0xf;
    j["query-no"] = qrn_;
    j["reply-no"] = rrn_;
    j["author-no"] = arn_;
    j["extra-no"] = ern_;

    if (qrn_ > 0) {
        Json query;
        for (auto const& q: query_) {
            query.push_back(toJsonQuery(q));
        }
        j["query"] = query;
    }

    if (rrn_ > 0) {
        Json reply;
        for (auto const& r: reply_) {
            reply.push_back(toJsonResource(r));
        }
        j["reply"] = reply;
    }

    if (arn_ > 0) {
        Json auth;
        for (auto const& a: auth_) {
            auth.push_back(toJsonResource(a));
        }
        j["auth"] = auth;
    }

    if (ern_ > 0) {
        Json extra;
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
    q["domain"] = query.domain;
    q["query-type"] = query.type;
    q["query-class"] = query.cls;
    return q;
}

Json Dns::toJsonResource(Resource const& res)
{
    Json r;
    r["domain"] = res.domain;
    r["query-type"] = res.type;
    r["query-class"] = res.cls;
    r["ttl"] = res.ttl;
    r["data-size"] = res.data.size();
    if (res.type == 1) {  // A
        r["data"] = reinterpret_cast<Ip4 const*>(res.data.data())->toStr();
    } else if (res.type == 5) {  // CNAME
        r["data"] = std::string(res.data.begin(), res.data.end());
    }
    return r;
}

Protocol::Type Dns::type() const { return kDNS; }

bool Dns::correlated(Protocol const& resp) const
{
    if (type() == resp.type()) {
        auto p = dynamic_cast<Dns const&>(resp);
        return id_ == p.id_;
    }
    return false;
}

void Dns::encodeQuery(BytesBuilder& builder, Query const& query)
{
    encodeDomain(builder, query.domain);
    builder.write16u(query.type);
    builder.write16u(query.cls);
}

Dns::Query Dns::decodeQuery(BytesReader& reader)
{
    Query query;
    query.domain = decodeDomain(reader);
    query.type = reader.read16u();
    query.cls = reader.read16u();
    return query;
}

void Dns::encodeResource(BytesBuilder& builder, Resource const& res)
{
    encodeDomain(builder, res.domain);
    builder.write16u(res.type);
    builder.write16u(res.cls);
    builder.write32u(res.ttl);
    builder.write16u(res.data.size());
    builder.writeBytes(res.data);
}

Dns::Resource Dns::decodeResource(BytesReader& reader)
{
    Resource res;
    res.domain = decodeDomain(reader);
    res.type = reader.read16u();
    res.cls = reader.read16u();
    res.ttl = reader.read32u();
    uint16_t dsize = reader.read16u();
    res.data = reader.readBytes(dsize);
    return res;
}

void Dns::encodeDomain(BytesBuilder& builder, std::string const& domain)
{
    std::vector<std::string> svec;
    boost::split(svec, domain, boost::is_any_of("."));
    for (auto const& s: svec) {
        builder.write8u(s.size());
        for (char c: s) {
            builder.write8u(c);
        }
    }
    builder.write8u(0);
}

static std::pair<std::string, size_t> iterDecodeDomain(uint8_t const* const pstart,
                                                       uint8_t const* const pend)
{
    std::vector<std::string> svec;
    uint8_t const* it = pstart;
    bool compressed = false;
    for (; it < pend && *it != 0;) {
        size_t cnt = *it;
        if ((cnt & 0xc0) != 0xc0) {
            svec.emplace_back(it + 1, it + cnt + 1);
            it += cnt + 1;
        } else {
            compressed = true;
            uint16_t index = ((cnt & 0x3f) << 8) | it[1];
            auto new_start = pstart + index;
            svec.push_back(iterDecodeDomain(new_start, pend).first);
            it += 2;
            break;
        }
    }
    if (!compressed) {
        ++it;
    }
    return std::make_pair(boost::join(svec, "."), std::distance(pstart, it));
}

std::string Dns::decodeDomain(BytesReader& reader)
{
    auto [domain, offset] = iterDecodeDomain(reader.data(), reader.data() + reader.size());
    reader.readBytes(offset);
    return domain;
}

}  // namespace net