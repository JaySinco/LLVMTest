#pragma once
#include "utils/logging.h"
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
    bool onSameLAN(Ip4 rhs, Ip4 mask) const;
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
};

struct Packet
{
    int64_t t_ms;
    std::vector<uint8_t> bytes;
};

template <typename T>
struct Tagged
{
    T v;
    int16_t beg = -1;
    int16_t end = -1;

    Json toJson() const;
};

class BytesReader
{
public:
    explicit BytesReader(std::vector<uint8_t> const& bytes);

    uint8_t const* data() const { return data_; }

    size_t size() const { return size_; }

    bool empty() const { return size_ == 0; }

    void read8u(Tagged<uint8_t>& b);
    void read16u(Tagged<uint16_t>& b, bool ntoh = true);
    void read32u(Tagged<uint32_t>& b, bool ntoh = true);
    void readIp4(Tagged<Ip4>& b);
    void readMac(Tagged<Mac>& b);
    void readDomain(Tagged<std::string>& b);
    void readBytes(Tagged<std::vector<uint8_t>>& b, size_t n);
    void readAll(Tagged<std::vector<uint8_t>>& b);
    void readAll(Tagged<std::string>& b);

private:
    const size_t total_size_;
    uint8_t const* data_;
    size_t size_;
};

class BytesBuilder
{
public:
    BytesBuilder() = default;

    uint8_t const* data() const { return data_.data(); }

    size_t size() const { return data_.size(); }

    void prependTo(std::vector<uint8_t>& bytes);

    void write8u(uint8_t b);
    void write16u(uint16_t b, bool hton = true);
    void write32u(uint32_t b, bool hton = true);
    void writeIp4(Ip4 b);
    void writeMac(Mac b);
    void writeDomain(std::string const& b);
    void writeBytes(uint8_t const* b, size_t size);
    void writeBytes(std::string const& b);
    void writeBytes(std::vector<uint8_t> const& b);

private:
    std::vector<uint8_t> data_;
};

}  // namespace net
