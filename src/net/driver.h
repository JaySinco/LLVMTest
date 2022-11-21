#pragma once
#include "protocol.h"

namespace net
{

class Error;
template <typename T>
using Expected = nonstd::expected<T, Error>;

class Driver
{
public:
    explicit Driver(Ip4 hint = Ip4::kNull, int to_ms = 1000);
    explicit Driver(Adaptor const& apt, int to_ms);
    ~Driver();

    ProtocolStack broadcastedARP(Mac smac, Ip4 sip, Mac dmac, Ip4 dip, bool reply = false) const;

    Expected<Mac> getMac(Ip4 ip, bool use_cache = false, int to_ms = 3000) const;

    Expected<ProtocolStack> request(ProtocolStack const& req, int to_ms = -1,
                                    bool recv_only = false) const;

    Expected<Packet> recv() const;
    void send(ProtocolStack const& stack) const;
    void send(Packet const& pac) const;

    Adaptor const& getAdaptor() const { return apt_; }

private:
    Adaptor apt_;
    void* p_;
};

class Error
{
public:
    static nonstd::unexpected_type<Error> unexpected(Error const& err);
    static nonstd::unexpected_type<Error> catchAll(std::string const& msg);
    static nonstd::unexpected_type<Error> timeout(std::string const& msg);
    static nonstd::unexpected_type<Error> packetExpired(std::string const& msg);

    bool catchAll() const { return flags_ & kCatchAll; }

    bool timeout() const { return flags_ & kTimeout; }

    bool packetExpired() const { return flags_ & kPacketExpired; }

    std::string const& what() const { return msg_; };

private:
    enum Type
    {
        kCatchAll = 1,
        kTimeout = 1 << 1,
        kPacketExpired = 1 << 2,
    };

    uint32_t flags_;
    std::string msg_;
};

}  // namespace net