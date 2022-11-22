#include "type.h"
#include "platform.h"
#include <sstream>
#include <regex>
#include <boost/algorithm/string.hpp>

namespace net
{

const Mac Mac::kNull = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
const Mac Mac::kBroadcast = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bool Mac::operator==(Mac const& rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4 && b5 == rhs.b5 &&
           b6 == rhs.b6;
}

bool Mac::operator!=(Mac const& rhs) const { return !(*this == rhs); }

std::string Mac::toStr() const
{
    auto c = reinterpret_cast<uint8_t const*>(this);
    std::ostringstream ss;
    ss << FSTR("{:02x}", c[0]);
    for (int i = 1; i < 6; ++i) {
        ss << FSTR("-{:02x}", c[i]);
    }
    return ss.str();
}

const Ip4 Ip4::kNull = {0x0, 0x0, 0x0, 0x0};
const Ip4 Ip4::kBroadcast = {0xff, 0xff, 0xff, 0xff};

Ip4 Ip4::fromDottedDec(std::string const& s)
{
    static std::regex pat(R"(^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$)");
    if (!regex_match(s, pat)) {
        MY_THROW("invalid ip4: {}", s);
    }
    std::vector<std::string> parts;
    boost::split(parts, s, boost::is_any_of("."));
    Ip4 ip;
    ip.b1 = std::stoi(parts[0]);
    ip.b2 = std::stoi(parts[1]);
    ip.b3 = std::stoi(parts[2]);
    ip.b4 = std::stoi(parts[3]);
    return ip;
}

Ip4::operator uint32_t() const
{
    auto i = reinterpret_cast<uint32_t const*>(this);
    return ntohl(*i);
}

bool Ip4::operator==(Ip4 const& rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4;
}

bool Ip4::operator!=(Ip4 const& rhs) const { return !(*this == rhs); }

bool Ip4::operator<(Ip4 const& rhs) const
{
    auto i = reinterpret_cast<uint32_t const*>(this);
    auto j = reinterpret_cast<uint32_t const*>(&rhs);
    return *i < *j;
}

uint32_t Ip4::operator&(Ip4 const& rhs) const
{
    return static_cast<uint32_t>(*this) & static_cast<uint32_t>(rhs);
}

bool Ip4::onSameLAN(Ip4 rhs, Ip4 mask) const { return (*this & mask) == (rhs & mask); }

bool Ip4::isSelf() const
{
    auto apts = allAdaptors();
    return std::find_if(apts.begin(), apts.end(),
                        [&](Adaptor const& apt) { return (*this) == apt.ip; }) != apts.end();
}

std::string Ip4::toStr() const
{
    auto c = reinterpret_cast<uint8_t const*>(this);
    std::ostringstream ss;
    ss << static_cast<int>(c[0]);
    for (int i = 1; i < 4; ++i) {
        ss << "." << static_cast<int>(c[i]);
    }
    return ss.str();
}

Json Adaptor::toJson() const
{
    Json j;
    j["name"] = name;
    j["desc"] = desc;
    j["mac"] = mac.toStr();
    j["ip"] = ip.toStr();
    j["mask"] = mask.toStr();
    j["gateway"] = gateway.toStr();
    return j;
}

BytesReader::BytesReader(std::vector<uint8_t> const& bytes)
    : total_size_(bytes.size()), data_(bytes.data()), size_(bytes.size())
{
}

#define CHECK_SIZE(n)                                                      \
    if (size_ < n) {                                                       \
        MY_THROW("inadequate bytes: {} expected, only {} left", n, size_); \
    }

void BytesReader::read8u(Tagged<uint8_t>& b)
{
    CHECK_SIZE(sizeof(uint8_t));

    b.v = *reinterpret_cast<uint8_t const*>(data_);
    b.beg = total_size_ - size_;
    b.end = b.beg + sizeof(uint8_t) - 1;

    data_ += sizeof(uint8_t);
    size_ -= sizeof(uint8_t);
}

void BytesReader::read16u(Tagged<uint16_t>& b, bool ntoh)
{
    CHECK_SIZE(sizeof(uint16_t));

    uint16_t temp = *reinterpret_cast<uint16_t const*>(data_);
    b.v = ntoh ? ntohs(temp) : temp;
    b.beg = total_size_ - size_;
    b.end = b.beg + sizeof(uint16_t) - 1;

    data_ += sizeof(uint16_t);
    size_ -= sizeof(uint16_t);
}

void BytesReader::read32u(Tagged<uint32_t>& b, bool ntoh)
{
    CHECK_SIZE(sizeof(uint32_t));

    uint32_t temp = *reinterpret_cast<uint32_t const*>(data_);
    b.v = ntoh ? ntohs(temp) : temp;
    b.beg = total_size_ - size_;
    b.end = b.beg + sizeof(uint32_t) - 1;

    data_ += sizeof(uint32_t);
    size_ -= sizeof(uint32_t);
}

void BytesReader::readIp4(Tagged<Ip4>& b)
{
    CHECK_SIZE(sizeof(Ip4));

    b.v = *reinterpret_cast<Ip4 const*>(data_);
    b.beg = total_size_ - size_;
    b.end = b.beg + sizeof(Ip4) - 1;

    data_ += sizeof(Ip4);
    size_ -= sizeof(Ip4);
}

void BytesReader::readMac(Tagged<Mac>& b)
{
    CHECK_SIZE(sizeof(Mac));

    b.v = *reinterpret_cast<Mac const*>(data_);
    b.beg = total_size_ - size_;
    b.end = b.beg + sizeof(Mac) - 1;

    data_ += sizeof(Mac);
    size_ -= sizeof(Mac);
}

static std::pair<std::string, size_t> decodeDomain(uint8_t const* const pbeg,
                                                   uint8_t const* const pend)
{
    std::vector<std::string> svec;
    uint8_t const* it = pbeg;
    bool compressed = false;
    for (; it < pend && *it != 0;) {
        size_t cnt = *it;
        if ((cnt & 0xc0) != 0xc0) {
            svec.emplace_back(it + 1, it + cnt + 1);
            it += cnt + 1;
        } else {
            compressed = true;
            uint16_t index = ((cnt & 0x3f) << 8) | it[1];
            auto new_start = pbeg + index;
            svec.push_back(decodeDomain(new_start, pend).first);
            it += 2;
            break;
        }
    }
    if (!compressed) {
        ++it;
    }
    return std::make_pair(boost::join(svec, "."), std::distance(pbeg, it));
}

void BytesReader::readDomain(Tagged<std::string>& b)
{
    auto&& [domain, tlen] = decodeDomain(data_, data_ + size_);
    b.v = domain;
    b.beg = total_size_ - size_;
    b.end = b.beg + tlen - 1;

    data_ += tlen;
    size_ -= tlen;
}

void BytesReader::readBytes(Tagged<std::vector<uint8_t>>& b, size_t n)
{
    CHECK_SIZE(n);

    b.v = std::vector<uint8_t>(data_, data_ + n);
    b.beg = total_size_ - size_;
    b.end = b.beg + n - 1;

    data_ += n;
    size_ -= n;
}

void BytesReader::readAll(Tagged<std::vector<uint8_t>>& b)
{
    b.v = std::vector<uint8_t>(data_, data_ + size_);
    b.beg = total_size_ - size_;
    b.end = b.beg + size_ - 1;

    data_ += size_;
    size_ = 0;
}

void BytesReader::readAll(Tagged<std::string>& b)
{
    b.v = std::string(data_, data_ + size_);
    b.beg = total_size_ - size_;
    b.end = b.beg + size_ - 1;

    data_ += size_;
    size_ = 0;
}

void BytesBuilder::prependTo(std::vector<uint8_t>& bytes)
{
    bytes.insert(bytes.begin(), data_.begin(), data_.end());
}

void BytesBuilder::write8u(uint8_t b)
{
    uint8_t* d = reinterpret_cast<uint8_t*>(&b);
    data_.insert(data_.end(), d, d + sizeof(uint8_t));
}

void BytesBuilder::write16u(uint16_t b, bool hton)
{
    uint16_t h = hton ? htons(b) : b;
    uint8_t* d = reinterpret_cast<uint8_t*>(&h);
    data_.insert(data_.end(), d, d + sizeof(uint16_t));
}

void BytesBuilder::write32u(uint32_t b, bool hton)
{
    uint32_t h = hton ? htonl(b) : b;
    uint8_t* d = reinterpret_cast<uint8_t*>(&h);
    data_.insert(data_.end(), d, d + sizeof(uint32_t));
}

void BytesBuilder::writeIp4(Ip4 b)
{
    uint8_t* d = reinterpret_cast<uint8_t*>(&b);
    data_.insert(data_.end(), d, d + sizeof(Ip4));
}

void BytesBuilder::writeMac(Mac b)
{
    uint8_t const* d = reinterpret_cast<uint8_t const*>(&b);
    data_.insert(data_.end(), d, d + sizeof(Mac));
}

void BytesBuilder::writeDomain(std::string const& b)
{
    std::vector<std::string> svec;
    boost::split(svec, b, boost::is_any_of("."));
    for (auto const& s: svec) {
        write8u(s.size());
        writeBytes(s);
    }
    write8u(0);
}

void BytesBuilder::writeBytes(uint8_t const* b, size_t size)
{
    data_.insert(data_.end(), b, b + size);
}

void BytesBuilder::writeBytes(std::string const& b)
{
    writeBytes(reinterpret_cast<uint8_t const*>(b.data()), b.size());
}

void BytesBuilder::writeBytes(std::vector<uint8_t> const& b) { writeBytes(b.data(), b.size()); }

template <>
Json Tagged<uint8_t>::toJson() const
{
    Json j;
    j["type"] = "uint8";
    j["value"] = v;
    j["begin"] = beg;
    j["end"] = end;
    return j;
}

template <>
Json Tagged<uint16_t>::toJson() const
{
    Json j;
    j["type"] = "uint16";
    j["value"] = v;
    j["begin"] = beg;
    j["end"] = end;
    return j;
}

template <>
Json Tagged<uint32_t>::toJson() const
{
    Json j;
    j["type"] = "uint32";
    j["value"] = v;
    j["begin"] = beg;
    j["end"] = end;
    return j;
}

template <>
Json Tagged<Ip4>::toJson() const
{
    Json j;
    j["type"] = "ip4";
    j["value"] = v.toStr();
    j["begin"] = beg;
    j["end"] = end;
    return j;
}

template <>
Json Tagged<Mac>::toJson() const
{
    Json j;
    j["type"] = "mac";
    j["value"] = v.toStr();
    j["begin"] = beg;
    j["end"] = end;
    return j;
}

template <>
Json Tagged<std::string>::toJson() const
{
    Json j;
    j["type"] = "str";
    j["value"] = v;
    j["begin"] = beg;
    j["end"] = end;
    return j;
}

template <>
Json Tagged<std::vector<uint8_t>>::toJson() const
{
    Json j;
    j["type"] = "svec";
    j["tlen"] = v.size();
    j["begin"] = beg;
    j["end"] = end;
    return j;
}

}  // namespace net
