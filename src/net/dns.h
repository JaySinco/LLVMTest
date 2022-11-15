#pragma once
#include "protocol.h"

class dns: public protocol
{
public:
    struct detail
    {
        u_short id;     // Identification
        u_short flags;  // Flags
        u_short qrn;    // Query number
        u_short rrn;    // Reply resource record number
        u_short arn;    // Auth resource record number
        u_short ern;    // Extra resource record number
    };

    struct query_detail
    {
        std::string domain;  // Query domain
        u_short type;        // Query type
        u_short cls;         // Query class
    };

    struct res_detail
    {
        std::string domain;  // Domain
        u_short type;        // Query type
        u_short cls;         // Query class
        u_int ttl;           // Time to live
        u_short dlen;        // Resource data length
        std::string data;    // Resource data
    };

    struct extra_detail
    {
        std::vector<query_detail> query;  // Query
        std::vector<res_detail> reply;    // Reply
        std::vector<res_detail> auth;     // Auth
        std::vector<res_detail> extra;    // Extra
    };

    dns() = default;

    dns(u_char const* const start, u_char const*& end, protocol const* prev = nullptr);

    dns(std::string const& query_domain);

    virtual ~dns() = default;

    virtual void to_bytes(std::vector<u_char>& bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(protocol const& rhs) const override;

    detail const& get_detail() const;

    extra_detail const& get_extra() const;

private:
    detail d{0};

    extra_detail extra;

    static std::string encode_domain(std::string const& domain);

    static std::string decode_domain(u_char const* const pstart, u_char const* const pend,
                                     u_char const*& it);

    static detail ntoh(detail const& d, bool reverse = false);

    static detail hton(detail const& d);
};
