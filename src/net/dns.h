#pragma once
#include "protocol.h"

namespace net
{

class Dns: public Protocol
{
public:
    Dns() = default;
    explicit Dns(BytesReader& reader);
    static Dns query(std::string const& domain);
    ~Dns() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    Type typeNext() const override;
    bool correlated(Protocol const& resp) const override;

private:
    DEFINE_PROP(uint16_t, id, "identification");
    DEFINE_PROP(uint16_t, flags, "flags");
    DEFINE_PROP(uint16_t, qrn, "query number");
    DEFINE_PROP(uint16_t, rrn, "reply resource record number");
    DEFINE_PROP(uint16_t, arn, "auth resource record number");
    DEFINE_PROP(uint16_t, ern, "extra resource record number");

    struct Query
    {
        DEFINE_PROP(std::string, domain, "domain");
        DEFINE_PROP(uint16_t, dm_type, "domain type");
        DEFINE_PROP(uint16_t, cls, "domain class");
    };

    struct Resource
    {
        DEFINE_PROP(std::string, domain, "domain");
        DEFINE_PROP(uint16_t, dm_type, "domain type");
        DEFINE_PROP(uint16_t, cls, "domain class");
        DEFINE_PROP(uint32_t, ttl, "time to live");
        DEFINE_PROP(uint16_t, dlen, "resource data size");
        DEFINE_PROP(std::vector<uint8_t>, data, "resource data");
    };

    std::vector<Query> query_;     // Query
    std::vector<Resource> reply_;  // Reply
    std::vector<Resource> auth_;   // Auth
    std::vector<Resource> extra_;  // Extra

    static constexpr size_t kFixedBytes = 12;

    static Json toJsonQuery(Query const& query);
    static Json toJsonResource(Resource const& res);

    static void encodeQuery(BytesBuilder& builder, Query const& query);
    static Query decodeQuery(BytesReader& reader);

    static void encodeResource(BytesBuilder& builder, Resource const& res);
    static Resource decodeResource(BytesReader& reader);
};

}  // namespace net