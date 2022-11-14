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

    static Ip4 fromDottedDec(std::string const& s);

    explicit operator uint32_t() const;
    bool operator==(Ip4 const& rhs) const;
    bool operator!=(Ip4 const& rhs) const;
    bool operator<(Ip4 const& rhs) const;
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

class BytesReader
{
public:
    explicit BytesReader(std::vector<uint8_t> const& bytes);
    bool empty() const;
    size_t size() const;

    uint8_t read8u();
    uint16_t read16u(bool ntoh = true);
    uint32_t read32u(bool ntoh = true);
    Ip4 readIp4();
    Mac readMac();

    std::vector<uint8_t> readBytes(size_t n);
    std::vector<uint8_t> readAll();

private:
    uint8_t const* data_;
    size_t size_;
};

class BytesBuilder
{
public:
    BytesBuilder();
    uint8_t const* data() const;
    size_t size() const;
    void prependTo(std::vector<uint8_t>& bytes);

    void write8u(uint8_t b);
    void write16u(uint16_t b, bool hton = true);
    void write32u(uint32_t b, bool hton = true);
    void writeIp4(Ip4 b);
    void writeMac(Mac const& b);
    void writeBytes(uint8_t const* data, size_t size);
    void writeBytes(std::vector<uint8_t> const& b);

private:
    std::vector<uint8_t> data_;
};

}  // namespace net
