#ifndef PROJECT_TYPES
#define PROJECT_TYPES
#include <stdint.h>

struct in6_memaddr {
    uint32_t wildcard;
    uint16_t subid;
    uint16_t cmd;
    uint64_t paddr;
};

struct pkt_rqst {
    struct in6_addr *dst_addr;
    int dst_port;
    char *data;
    int datalen;
};


// Define some constants.
#define ETH_HDRLEN 14  // Ethernet header length
#define IP6_HDRLEN 40  // IPv6 header length
#define UDP_HDRLEN 8  // UDP header length, excludes data
#define IPV6_SIZE 16
#define BLOCK_SIZE 4096

#define ALLOC_CMD       01
#define WRITE_CMD       02
#define GET_CMD         03
#define FREE_CMD        04
#define GET_ADDR_CMD    05
#define ALLOC_BULK_CMD  06
#define CMD_SIZE        02

/*          struct sockaddr_in6 {
               sa_family_t     sin6_family;   AF_INET6
               in_port_t       sin6_port;     port number
               uint32_t        sin6_flowinfo; IPv6 flow information
               struct in6_addr sin6_addr;     IPv6 address
               uint32_t        sin6_scope_id; Scope ID (new in 2.4)};
*/
/// Offset of data from start of frame
#define PKT_OFFSET      (TPACKET_ALIGN(sizeof(struct tpacket_hdr)) + \
                         TPACKET_ALIGN(sizeof(struct sockaddr_ll)))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#endif
