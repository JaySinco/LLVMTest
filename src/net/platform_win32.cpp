#include "platform.h"
#define UNICODE
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>

namespace net
{

struct WSAGuard
{
    WSAGuard()
    {
        WSADATA ws;
        WSAStartup(MAKEWORD(2, 2), &ws);
    }

    ~WSAGuard() { WSACleanup(); }

private:
    static WSAGuard g;
};

WSAGuard WSAGuard::g;

uint32_t htonl(uint32_t hostlong) { return ::htonl(hostlong); }

uint16_t htons(uint16_t hostshort) { return ::htons(hostshort); }

uint32_t ntohl(uint32_t netlong) { return ::ntohl(netlong); }

uint16_t ntohs(uint16_t netshort) { return ::ntohs(netshort); }

static Ip4 ip4FromInAddr(in_addr addr)
{
    Ip4 ip;
    ip.b1 = addr.S_un.S_un_b.s_b1;
    ip.b2 = addr.S_un.S_un_b.s_b2;
    ip.b3 = addr.S_un.S_un_b.s_b3;
    ip.b4 = addr.S_un.S_un_b.s_b4;
    return ip;
}

static in_addr ip4ToInAddr(Ip4 ip)
{
    in_addr addr;
    addr.S_un.S_un_b.s_b1 = ip.b1;
    addr.S_un.S_un_b.s_b2 = ip.b2;
    addr.S_un.S_un_b.s_b3 = ip.b3;
    addr.S_un.S_un_b.s_b4 = ip.b4;
    return addr;
}

std::vector<Adaptor> const& allAdaptors()
{
    static std::once_flag flag;
    static std::vector<Adaptor> adapters;

    std::call_once(flag, [&] {
        ULONG buflen = sizeof(IP_ADAPTER_INFO);
        auto plist = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(sizeof(IP_ADAPTER_INFO)));
        auto plist_guard = utils::scopeExit([&]() { free(plist); });
        if (GetAdaptersInfo(plist, &buflen) == ERROR_BUFFER_OVERFLOW) {
            plist = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buflen));
            if (GetAdaptersInfo(plist, &buflen) != NO_ERROR) {
                MY_THROW("failed to get adapters info");
            }
        }
        PIP_ADAPTER_INFO pinfo = plist;
        while (pinfo) {
            Adaptor apt;
            Ip4 ip = Ip4::fromDottedDec(pinfo->IpAddressList.IpAddress.String);
            if (ip != Ip4::kNull) {
                apt.name = std::string("\\Device\\NPF_") + pinfo->AdapterName;
                apt.desc = pinfo->Description;
                apt.ip = ip;
                apt.mask = Ip4::fromDottedDec(pinfo->IpAddressList.IpMask.String);
                apt.gateway = Ip4::fromDottedDec(pinfo->GatewayList.IpAddress.String);
                if (pinfo->AddressLength != sizeof(Mac)) {
                    WLOG("incompatible mac length: {}", pinfo->AddressLength);
                    continue;
                }
                auto c = reinterpret_cast<uint8_t*>(&apt.mac);
                for (unsigned i = 0; i < pinfo->AddressLength; ++i) {
                    c[i] = pinfo->Address[i];
                }
                adapters.push_back(apt);
            }
            pinfo = pinfo->Next;
        }
        if (adapters.size() <= 0) {
            MY_THROW("failed to find any suitable adapter");
        }
    });

    return adapters;
}

static std::string pidToImageName(uint32_t pid, int to_sec = 60)
{
    static std::map<uint32_t, std::pair<std::string, std::chrono::system_clock::time_point>> cached;
    auto now = std::chrono::system_clock::now();
    auto it = cached.find(pid);
    if (it != cached.end() && now - it->second.second < std::chrono::seconds(to_sec)) {
        return it->second.first;
    }

    std::string image = FSTR("pid({})", pid);
    HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (handle == nullptr) {
        return image;
    }
    auto handle_guard = utils::scopeExit([&]() { CloseHandle(handle); });
    char buf[1024];
    DWORD size = sizeof(buf);
    if (!QueryFullProcessImageNameA(handle, 0, buf, &size)) {
        return image;
    }
    std::filesystem::path fp(std::string(buf, size));
    image = fp.filename().string();
    cached[pid] = std::make_pair(image, now);
    return image;
}

static void tcpPortToImageSnapshot(std::map<PortMappingKey, std::string>& mappping)
{
    ULONG size = sizeof(MIB_TCPTABLE);
    PMIB_TCPTABLE2 ptable = reinterpret_cast<MIB_TCPTABLE2*>(malloc(size));
    auto ptable_guard = utils::scopeExit([&]() { free(ptable); });
    DWORD ret = 0;
    if ((ret = GetTcpTable2(ptable, &size, FALSE)) == ERROR_INSUFFICIENT_BUFFER) {
        free(ptable);
        ptable = reinterpret_cast<MIB_TCPTABLE2*>(malloc(size));
        if (ptable == nullptr) {
            MY_THROW("failed to allocate memory, size={}", size);
        }
    }
    ret = GetTcpTable2(ptable, &size, FALSE);
    if (ret != NO_ERROR) {
        MY_THROW("failed to get tcp table, ret={}", ret);
    }
    for (int i = 0; i < ptable->dwNumEntries; ++i) {
        in_addr addr;
        addr.S_un.S_addr = ptable->table[i].dwLocalAddr;
        Ip4 ip = ip4FromInAddr(addr);
        uint16_t port = ntohs(ptable->table[i].dwLocalPort);
        uint32_t pid = ptable->table[i].dwOwningPid;
        if (pid != 0) {
            mappping[std::make_tuple(Protocol::kTCP, ip, port)] = pidToImageName(pid);
        }
    }
}

static void udpPortToImageSnapshot(std::map<PortMappingKey, std::string>& mappping)
{
    ULONG size = sizeof(MIB_UDPTABLE_OWNER_PID);
    PMIB_UDPTABLE_OWNER_PID ptable = reinterpret_cast<MIB_UDPTABLE_OWNER_PID*>(malloc(size));
    auto ptable_guard = utils::scopeExit([&]() { free(ptable); });
    DWORD ret = 0;
    if ((ret = GetExtendedUdpTable(ptable, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0)) ==
        ERROR_INSUFFICIENT_BUFFER) {
        free(ptable);
        ptable = reinterpret_cast<MIB_UDPTABLE_OWNER_PID*>(malloc(size));
        if (ptable == nullptr) {
            MY_THROW("failed to allocate memory, size={}", size);
        }
    }
    ret = GetExtendedUdpTable(ptable, &size, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
    if (ret != NO_ERROR) {
        MY_THROW("failed to get udp table, ret={}", ret);
    }
    for (int i = 0; i < ptable->dwNumEntries; ++i) {
        in_addr addr;
        addr.S_un.S_addr = ptable->table[i].dwLocalAddr;
        Ip4 ip = ip4FromInAddr(addr);
        uint16_t port = ntohs(ptable->table[i].dwLocalPort);
        uint32_t pid = ptable->table[i].dwOwningPid;
        if (pid != 0) {
            mappping[std::make_tuple(Protocol::kUDP, ip, port)] = pidToImageName(pid);
        }
    }
}

void portToImageNameSnapshot(std::map<PortMappingKey, std::string>& mappping)
{
    tcpPortToImageSnapshot(mappping);
    udpPortToImageSnapshot(mappping);
}

}  // namespace net