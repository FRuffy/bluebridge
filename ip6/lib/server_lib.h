#ifndef PROJECT_SERVER
#define PROJECT_SERVER
#include "types.h"
#include "network.h"

int allocate_mem(struct sockaddr_in6 *target_ip);
int allocate_mem_bulk(struct sockaddr_in6 *target_ip, uint64_t size);
int get_mem(struct sockaddr_in6 *target_ip, struct in6_memaddr *r_addr);
int write_mem(char * receiveBuffer, struct sockaddr_in6 *target_ip, struct in6_memaddr *r_addr);
int free_mem(struct sockaddr_in6 *target_ip, struct in6_memaddr *r_addr);
void process_request(char *receiveBuffer, struct sockaddr_in6 *targetIP, struct in6_memaddr *remoteAddr);
#endif