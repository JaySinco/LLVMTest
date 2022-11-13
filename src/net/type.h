#pragma once
#include "utils/base.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace net
{

using Json = nlohmann::ordered_json;

struct Mac
{
    uint8_t b1, b2, b3, b4, b5, b6;

    static const Mac kNull;
    static const Mac kBroadcast;

    bool operator==(Mac const& rhs) const;
    bool operator!=(Mac const& rhs) const;
    std::string toStr() const;
};

struct Ip4
{
    uint8_t b1, b2, b3, b4;

    static const Ip4 kNull;
    static const Ip4 kBroadcast;

    explicit operator uint32_t() const;
    bool operator==(Ip4 const& rhs) const;
    bool operator!=(Ip4 const& rhs) const;
    uint32_t operator&(Ip4 const& rhs) const;
    bool onSameLAN(Ip4 const& rhs, Ip4 const& mask) const;
    bool isSelf() const;
    std::string toStr() const;
};

struct Adaptor
{
    std::string name;
    std::string desc;
    Ip4 ip;
    Ip4 mask;
    Ip4 gateway;
    Mac mac;

    Json toJson() const;
    static Adaptor const& fit(Ip4 const& hint = Ip4::kNull);
};

struct Packet
{
    int64_t t_ms;
    std::vector<uint8_t> bytes;
};

}  // namespace net
