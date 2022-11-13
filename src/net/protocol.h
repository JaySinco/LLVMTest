#pragma once
#include "type.h"
#define ntohx(field, reverse, suffix) field = ((reverse) ? hton##suffix : ntoh##suffix)(field);

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
    };

    virtual ~Protocol() = default;

    virtual void toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const = 0;
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
    static ProtocolStack fromPacket(Packet const& pac);
    Packet toPacket() const;
    Json toJson() const;
    bool correlated(ProtocolStack const& resp) const;

    ProtocolPtr get(size_t idx) const { return stack_.at(idx); }

    ProtocolPtr get(Protocol::Type type) const;
    size_t getIdx(Protocol::Type type) const;

    void push(ProtocolPtr p) { stack_.push_back(p); }

    size_t size() const { return stack_.size(); };

private:
    std::vector<ProtocolPtr> stack_;
};

class Unimplemented: public Protocol
{
public:
    ~Unimplemented() override = default;

    static void fromBytes(uint8_t const*& data, size_t& size, ProtocolStack& stack);

    void toBytes(std::vector<uint8_t>& bytes, ProtocolStack const& stack) const override;
    Json toJson() const override;
    Type type() const override;
    bool correlated(Protocol const& resp) const override;
};

}  // namespace net