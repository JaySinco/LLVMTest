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
    bool correlated(Protocol const& resp) const override;

private:
    uint16_t id_;     // Identification
    uint16_t flags_;  // Flags
    uint16_t qrn_;    // Query number
    uint16_t rrn_;    // Reply resource record number
    uint16_t arn_;    // Auth resource record number
    uint16_t ern_;    // Extra resource record number

    struct Query
    {
        std::string domain;  // Query domain
        uint16_t type;       // Query type
        uint16_t cls;        // Query class
    };

    struct Resource
    {
        std::string domain;         // Domain
        uint16_t type;              // Query type
        uint16_t cls;               // Query class
        uint32_t ttl;               // Time to live
        std::vector<uint8_t> data;  // Resource data
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

    static void encodeDomain(BytesBuilder& builder, std::string const& domain);
    static std::string decodeDomain(BytesReader& reader);
};

}  // namespace net