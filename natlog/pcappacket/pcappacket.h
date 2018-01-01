#ifndef INCLUDED_PCAPPACKET_
#define INCLUDED_PCAPPACKET_

#include <pcap.h>

#include <arpa/inet.h>
#include <bobcat/inetaddress>

class PcapPacket
{
    struct pcap_pkthdr const &d_hdr;    // struct timeval: see gettimeofday(2)
    u_char const *d_packet;             // the captured packet

    struct Ethernet_Header      // http://en.wikipedia.org/wiki/EtherType
    {
        u_char destMac[6];      // Destination host MAC address
        u_char srcMac[6];       // Source host MAC address
        u_short ether_type;     // IP? ARP? RARP? etc 
    };

    enum class IP_Fragment
    {
        RESERVED =          0x8000,
        DONT_FRAGMENT =     0x4000,
        MORE_FRAGMENTS =    0x2000,
        MASK =              0x1fff
    };
    struct IP_Header 
    {
        u_char  hdrLength:4,        // in 4-byte words
                version:4;
        u_char tos;                 // type of service
        u_short length;             // total length 
        u_short identification;
        u_short fragmentOffset;
        u_char  timeToLive;
        u_char  protocol;
        u_short checkSum;
        struct in_addr sourceAddr;
        struct in_addr destAddr;
    };

    struct TCP_Header 
    {
        u_short sourcePort;
        u_short destPort;
        uint32_t sequenceNr;
        uint32_t ackNumber;

        u_char dataOffset;          // data offset, 
                                    // (((th)->th_offx2 & 0xf0) >> 4)
        u_char flags;
        u_short window;
        u_short checkSum;
        u_short urgentPtr;
    };

    struct UDP_Header               // udp header length: 8 bytes
    {
         u_short sourcePort;
         u_short destPort;
         u_short length;
         u_short checkSum;
    };                              

    enum Offsets                // offsets to the various headers
    {                           // the Ethernet header starts at `d_packet'
        ETHER_OFFSET =  0,       
        IP_OFFSET =     ETHER_OFFSET + sizeof(Ethernet_Header),
        TCP_OFFSET =    IP_OFFSET + sizeof(IP_Header),
        UDP_OFFSET =    TCP_OFFSET,
    };

    public:
        enum SizeofTCPheader
        {
            SIZEOF_ETHERNET_HEADER = IP_OFFSET,
        };

        enum Protocol
        {
            ICMP = 1,
            TCP = 6,
            UDP = 17
        };

        enum TCP_Flags
        {
            FIN  = 0x01,
            SYN  = 0x02,
            RST  = 0x04,
            PUSH = 0x08,
            ACK  = 0x10,
            URG  = 0x20,
            ECE  = 0x40,
            CWR  = 0x80,
            TCP_Flags_MASK = FIN | SYN | RST | ACK | URG | ECE | CWR
        };
        struct Address: public FBB::InetAddress
        {
            Address(struct in_addr const &addr, u_short port);
        };

        PcapPacket(struct pcap_pkthdr const &hdr, u_char const *packet);

        pcap_pkthdr const &timeval() const;

        time_t seconds() const;
        suseconds_t microSeconds() const;

        struct in_addr const &sourceAddr() const;
        struct in_addr const &destAddr() const;

        u_short sourcePort() const;
        u_short destPort() const;
  
        Address sourceIP() const;
        Address destIP() const;

        TCP_Flags flags() const;
        bool flags(u_char testFlags) const;

        uint32_t sequenceNr() const;

        size_t ipLength() const;
        size_t payloadLength() const;
        size_t dataOffset() const;
        size_t headerLength() const;
        size_t udpLength() const;

        size_t protocol() const;

    private:
        template <typename Type>
        Type const &get() const;

        Address inetAddr(struct in_addr const &addr, u_short port) const;
};

template <>
inline PcapPacket::IP_Header const &PcapPacket::get() const
{
    return *reinterpret_cast<IP_Header const *>(d_packet + IP_OFFSET);
}

template <>
inline PcapPacket::TCP_Header const &PcapPacket::get() const
{
    return *reinterpret_cast<TCP_Header const *>(d_packet + TCP_OFFSET);
}

template <>
inline PcapPacket::UDP_Header const &PcapPacket::get() const
{
    return *reinterpret_cast<UDP_Header const *>(d_packet + UDP_OFFSET);
}

inline PcapPacket::TCP_Flags PcapPacket::flags() const
{
    return static_cast<TCP_Flags>(get<TCP_Header>().flags);
}

inline PcapPacket::Address::Address(struct in_addr const &addr, u_short port)
:
    FBB::InetAddress( sockaddr_in{0, port, addr} )
{}

inline size_t PcapPacket::protocol() const
{
    return get<IP_Header>().protocol;
}

inline bool PcapPacket::flags(u_char testFlags) const
{
    return get<TCP_Header>().flags == testFlags;
}

inline pcap_pkthdr const &PcapPacket::timeval() const
{
    return d_hdr;
}
        
inline time_t PcapPacket::seconds() const
{
    return d_hdr.ts.tv_sec;
}
        
inline suseconds_t PcapPacket::microSeconds() const
{
    return d_hdr.ts.tv_usec;
}

inline struct in_addr const &PcapPacket::sourceAddr() const
{
    return get<IP_Header>().sourceAddr;
}

inline struct in_addr const &PcapPacket::destAddr() const
{
    return get<IP_Header>().destAddr;
}

inline u_short PcapPacket::sourcePort() const       // also works for UDP
{
    return get<TCP_Header>().sourcePort;
}

inline u_short PcapPacket::destPort() const         // also works for UDP
{
    return get<TCP_Header>().destPort;
}


inline PcapPacket::Address PcapPacket::sourceIP() const
{
    return 
        inetAddr(get<IP_Header>().sourceAddr, get<TCP_Header>().sourcePort);
}

inline PcapPacket::Address PcapPacket::destIP() const
{
    return 
        inetAddr(get<IP_Header>().destAddr, get<TCP_Header>().destPort);
}

inline PcapPacket::Address PcapPacket::inetAddr(struct in_addr const &addr, 
                                                u_short port) const
{
    return Address(addr, port);
}

inline uint32_t PcapPacket::sequenceNr() const
{
    return ntohl(get<TCP_Header>().sequenceNr);
}

inline size_t PcapPacket::dataOffset() const
{
            // 4 most significant bits: # 32 bit words
            // so: shr 4 to het the # 32 bit words, and 
            // << 2 to multiply by 4 to get the #bytes before the data
    return get<TCP_Header>().dataOffset >> 4 << 2;
}

inline size_t PcapPacket::headerLength() const
{
    return get<IP_Header>().hdrLength << 2;
}
        
inline size_t PcapPacket::udpLength() const
{
    return ntohs(get<UDP_Header>().length) - sizeof(UDP_Header); 
}
        
#endif
