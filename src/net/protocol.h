#pragma once
#include "type.h"

namespace net
{

class ProtocolStack;

class Protocol
{
public:
    enum Type
    {
        kUnknown,
        kEthernet,
        kIPv4,
        kIPv6,
        kARP,
        kRARP,
        kICMP,
        kTCP,
        kUDP,
        kDNS,
        kHTTP,
        kHTTPS,
        kSSH,
        kTelnet,
        kRDP,
    };

    virtual ~Protocol() = default;

    virtual void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const = 0;
    virtual Json toJson() const = 0;
    virtual Type type() const = 0;
    virtual bool correlated(Protocol const& resp) const = 0;

    static std::string descType(Type type);

protected:
    static uint16_t checksum(void const* data, size_t size);
    static uint16_t rand16u();
};

using ProtocolPtr = std::shared_ptr<Protocol>;

class ProtocolStack
{
public:
    static ProtocolStack decode(Packet const& pac);
    Packet encode() const;
    Json toJson() const;
    bool correlated(ProtocolStack const& resp) const;

    ProtocolPtr get(size_t idx) const { return stack_.at(idx); }

    ProtocolPtr get(Protocol::Type type) const;
    size_t getIdx(Protocol::Type type) const;

    bool has(Protocol::Type type) const;

    void push(ProtocolPtr p) { stack_.push_back(p); }

    size_t size() const { return stack_.size(); };

private:
    std::vector<ProtocolPtr> stack_;
};

class Unimplemented: public Protocol
{
public:
    ~Unimplemented() override = default;

    static void decode(BytesReader& reader, ProtocolStack& stack);
    void encode(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;

private:
    std::vector<uint8_t> buf_;  // leftover data
};

}  // namespace net