//
// Created by qlu on 2019/7/25.
//

#ifndef KUNGFU_SOCKET_OS_H
#define KUNGFU_SOCKET_OS_H

#if defined ( __linux__ )
    #define SYS_LINUX
#elif ( (!defined(__linux__)) && ( defined(__unix__) || ( defined(__APPLE__) && defined(__MACH__) ) || defined(__QNX__) ) )
    #define SYS_BSD
#elif ( defined(_WIN32) || defined(_WIN64) )
    #define SYS_NT
#endif

#if defined(SYS_LINUX) || defined(SYS_BSD)
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <errno.h>
    #include <err.h>
    #include <signal.h>

    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/if_ether.h>
    #include <netinet/tcp.h>
    #include <netinet/udp.h>
    #include <netdb.h>
    #include <net/if.h>
    #include <netinet/ip.h>
    #include <net/ethernet.h>

    #include <pcap.h>

    #if defined(SYS_LINUX)
        inline uint16_t source_port(const struct tcphdr* header) { return ntohs(header->source); }
        inline uint16_t dest_port(const struct tcphdr* header) { return ntohs(header->dest); }
        inline uint16_t source_port(const struct udphdr* header) { return ntohs(header->source); }
        inline uint16_t dest_port(const struct udphdr* header) { return ntohs(header->dest); }
        inline uint16_t len(const struct udphdr* header) { return ntohs(header->len); }
    #elif defined(SYS_BSD)
    inline uint16_t source_port(const struct tcphdr* header) { return ntohs(header->th_sport); }
        inline uint16_t dest_port(const struct tcphdr* header) { return ntohs(header->th_dport); }
        inline uint16_t source_port(const struct udphdr* header) { return ntohs(header->uh_sport); }
        inline uint16_t dest_port(const struct udphdr* header) { return ntohs(header->uh_dport); }
        inline uint16_t len(const struct udphdr* header) { return ntohs(header->uh_ulen); }
    #endif
#endif



#endif //KUNGFU_SOCKET_OS_H
