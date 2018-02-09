/*  Copyright (C) 2011-2015  P.D. Buchan (pdbuchan@yahoo.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PROJECT_COOK
#define PROJECT_COOK

// Send an IPv6 UDP packet via raw socket at the link layer (ethernet frame).
// Need to have destination MAC address.
// Includes some UDP data.


// Define some constants.
#define ETH_HDRLEN 14  // Ethernet header length
#define IP6_HDRLEN 40  // IPv6 header length
#define UDP_HDRLEN 8  // UDP header length, excludes data
#define IPV6_SIZE 16


/// The number of frames in the ring
//  This number is not set in stone. Nor are block_size, block_nr or frame_size
#define C_RING_FRAMES        16384 //8192
#define C_RING_BLOCKS        1
#define C_FRAMESIZE               8192//(4096 + ETH_HDRLEN + IP6_HDRLEN + UDP_HDRLEN + 2 + 32)
#define C_BLOCKSIZE               (C_FRAMESIZE) * (C_RING_FRAMES)


#include <stdint.h>         // needed for uint8_t, uint16_t
#include <netinet/ip6.h>    // struct ip6_hdr

#include "config.h"

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

extern int cooked_send(struct pkt_rqst pkt);
extern int cooked_batched_send(struct pkt_rqst *pkts, int num_pkts);
extern void init_send_socket(struct config *configstruct);

extern int get_send_socket();
extern int close_send_socket();

extern struct sockaddr_in6 *init_rcv_socket(struct config *configstruct);
extern int get_rcv_socket();
extern int epoll_rcv(char *receiveBuffer, int msgBlockSize, struct sockaddr_in6 *targetIP, struct in6_memaddr *remoteAddr, int server);
extern void init_epoll();

extern void close_rcv_socket();
extern void set_thread_id_tx(int id);
extern void set_thread_id_rx(int id);

extern void init_dpdk(struct config *configstruct);
extern int dpdk_send(struct pkt_rqst pkt);
extern int dpdk_rcv(char *receiveBuffer, int msgBlockSize, struct sockaddr_in6 *targetIP, struct in6_memaddr *remoteAddr, int server);

#endif