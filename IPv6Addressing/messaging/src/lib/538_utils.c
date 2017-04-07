#define _GNU_SOURCE


#include "538_utils.h"
#include "debug.h"

/* 
 * get sockaddr, IPv4 or IPv6:
 */
void *get_in_addr(struct sockaddr *sa) {
	// socket family is IPv4
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	// socket family is IPv6
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

/*
 * Gets the line from the command prompt
 * http://stackoverflow.com/questions/4023895/how-to-read-string-entered-by-user-in-c
 */
int getLine(char *prmpt, char *buff, size_t sz) {
	int ch, extra;

	// Get line with buffer overrun protection.
	if (prmpt != NULL) {
		printf("%s", prmpt);
		fflush(stdout);
	}
	if (fgets(buff, sz, stdin) == NULL)
		return 0;

	// If it was too long, there'll be no newline. In that case, we flush
	// to end of line so that excess doesn't affect the next call.
	if (buff[strlen(buff) - 1] != '\n') {
		extra = 0;
		while (((ch = getchar()) != '\n') && (ch != EOF))
			extra = 1;
		return (extra == 1) ? 0 : 1;
	}

	// Otherwise remove newline and give string back to caller.
	buff[strlen(buff) - 1] = '\0';
	return 1;
}

/*
 * Gets random byte array with size num_bytes
 */
unsigned char *gen_rdm_bytestream(size_t num_bytes) {
	unsigned char *stream = (unsigned char *) malloc(num_bytes);
	size_t i;

	for (i = 0; i < num_bytes; i++) {
		stream[i] = rand();
	}

	return stream;
}
/*
 * Gets random byte array with size num_bytes
 */
struct in6_addr * gen_rdm_IPv6Target() {
	// Add the pointer

	struct in6_addr * newAddr = (struct in6_addr *) calloc(1,sizeof(struct in6_addr));
	unsigned char * rndBytes = gen_rdm_bytestream(4);
	memcpy(newAddr->s6_addr+6,SUBNET_ID,2);
	memcpy(newAddr->s6_addr+8,rndBytes,4);


	char s[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6,newAddr, s, sizeof s);
	print_debug("Target IPv6 Pointer %s",s);
	free(rndBytes);
	return newAddr;
}

char * get_rdm_string(size_t num_bytes, int index) {
	char* stream = (char *) malloc(num_bytes);
	char *string = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";

	sprintf(stream, "%d:", index);

	size_t i = strlen(stream);
	for (; i < num_bytes; i++) {
		stream[i] = string[rand()%strlen(string)];
	}
	return stream;
}

/*
 * Sends message to specified socket
 */
//TODO: Remove?
int sendTCP(int sockfd, char * sendBuffer, int msgBlockSize) {
	if (send(sockfd, sendBuffer, msgBlockSize, 0) < 0) {
		perror("ERROR writing to socket");
		return EXIT_FAILURE;
	}
	memset(sendBuffer, 0, msgBlockSize);
	return EXIT_SUCCESS;
}

/*
 * Receives message from socket
 */
//TODO: Remove?
int receiveTCP(int sockfd, char * receiveBuffer, int msgBlockSize) {
	//Sockets Layer Call: recv()
	int numbytes = 0;
	memset(receiveBuffer, 0, msgBlockSize);
	if ((numbytes = recv(sockfd, receiveBuffer, msgBlockSize, 0)) == -1) {
		perror("ERROR reading from socket");
		exit(1);
	}
	return numbytes;
}
/*
 * Sends message to specified socket
 * Simpler version where we do not need the fancy to insert the IPv6Addr into the header
 */
int sendUDP(int sockfd, char * sendBuffer, int msgBlockSize, struct addrinfo * p) {
	char s[INET6_ADDRSTRLEN];
	//wait for incoming connection
	inet_ntop(p->ai_family,get_in_addr(p->ai_addr), s, sizeof s);
	socklen_t slen = sizeof(struct sockaddr_in6);
	print_debug("Sending to %s:%d", s,ntohs(((struct sockaddr_in6*) p->ai_addr)->sin6_port));
	if (sendto(sockfd,sendBuffer,msgBlockSize,0, p->ai_addr, slen) < 0) {
		perror("ERROR writing to socket");
		return EXIT_FAILURE;
	}
	memset(sendBuffer, 0, msgBlockSize);
	return EXIT_SUCCESS;
}
/*
 * Sends message to specified socket
 */
int sendUDPIPv6(int sockfd, char * sendBuffer, int msgBlockSize, struct addrinfo * p, struct in6_addr remotePointer) {
	
	char s[INET6_ADDRSTRLEN];
	inet_ntop(p->ai_family,get_in_addr(p->ai_addr), s, sizeof s);
	print_debug("Previous pointer... %s:%d",s,ntohs(((struct sockaddr_in6*) p->ai_addr)->sin6_port) );
	
	memcpy(&(((struct sockaddr_in6*) p->ai_addr)->sin6_addr), &remotePointer, sizeof(remotePointer));
	p->ai_addrlen = sizeof(remotePointer);
	
	inet_ntop(p->ai_family,get_in_addr(p->ai_addr), s, sizeof s);
	print_debug("Inserting %u Pointer into packet header... %s:%d",p->ai_addrlen,s,ntohs(((struct sockaddr_in6*) p->ai_addr)->sin6_port) );
	
	
	socklen_t slen = sizeof(struct sockaddr_in6);
	if (sendto(sockfd,sendBuffer,msgBlockSize,0, p->ai_addr, slen) < 0) {
		perror("ERROR writing to socket");
		return EXIT_FAILURE;
	}
	memset(sendBuffer, 0, msgBlockSize);
	return EXIT_SUCCESS;
}


//TODO: Remove?
/*
 * Receives message from socket
 */
int receiveUDPLegacy(int sockfd, char * receiveBuffer, int msgBlockSize, struct addrinfo * p) {
	int numbytes = 0;
	char s[INET6_ADDRSTRLEN];
	socklen_t slen = sizeof(struct sockaddr_in6);

	memset(receiveBuffer, 0, msgBlockSize);
	if ((numbytes = recvfrom(sockfd,receiveBuffer, msgBlockSize, 0, p->ai_addr,&slen)) == -1) {
		perror("ERROR reading from socket");
		exit(1);
	}
	//wait for incoming connection
	inet_ntop(p->ai_family,(struct sockaddr *) get_in_addr(p->ai_addr), s, sizeof s);
	printf("Got message from %s:%d \n", s,ntohs(((struct sockaddr_in6*) p->ai_addr)->sin6_port));
	return numbytes;
}

/*
 * Receives message from socket
 */
 //http://stackoverflow.com/questions/3062205/setting-the-source-ip-for-a-udp-socket
int receiveUDPIPv6(int sockfd, char * receiveBuffer, int msgBlockSize, struct addrinfo * p, struct in6_addr * ipv6Pointer) {

	struct sockaddr_in6 from;
	struct iovec iovec[1];
	struct msghdr msg;
	char msg_control[1024];
	char udp_packet[msgBlockSize];
	int numbytes = 0;
	char s[INET6_ADDRSTRLEN];
	iovec[0].iov_base = udp_packet;
	iovec[0].iov_len = sizeof(udp_packet);
	msg.msg_name = &from;
	msg.msg_namelen = sizeof(from);
	msg.msg_iov = iovec;
	msg.msg_iovlen = sizeof(iovec) / sizeof(*iovec);
	msg.msg_control = msg_control;
	msg.msg_controllen = sizeof(msg_control);
	msg.msg_flags = 0;

	numbytes = recvmsg(sockfd, &msg, 0);
	struct in6_pktinfo * in6_pktinfo;
	struct cmsghdr* cmsg;

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != 0; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_PKTINFO) {
			in6_pktinfo = (struct in6_pktinfo*)CMSG_DATA(cmsg);
			inet_ntop(p->ai_family,&in6_pktinfo->ipi6_addr, s, sizeof s);
			print_debug("Received packet was sent to this IP %s",s);
			memcpy(ipv6Pointer->s6_addr,&in6_pktinfo->ipi6_addr,IPV6_SIZE);
			memcpy(receiveBuffer,iovec[0].iov_base,iovec[0].iov_len);
			memcpy(p->ai_addr, (struct sockaddr *) &from, sizeof(from));
			p->ai_addrlen = sizeof(from);
		}
	}

	inet_ntop(p->ai_family,(struct sockaddr *) get_in_addr(p->ai_addr), s, sizeof s);
	printf("Got message from %s:%d \n", s,ntohs(((struct sockaddr_in6*) p->ai_addr)->sin6_port));

	return numbytes;
}



/*
 * Receives message from socket
 * Simpler version where we do not need the fancy msghdr structure
 */
int receiveUDP(int sockfd, char * receiveBuffer, int msgBlockSize, struct addrinfo * p) {

	int numbytes = 0;
	socklen_t slen = sizeof(struct sockaddr_in6);

	memset(receiveBuffer, 0, msgBlockSize);
	if ((numbytes = recvfrom(sockfd,receiveBuffer, msgBlockSize, 0, p->ai_addr,&slen)) == -1) {
		perror("ERROR reading from socket");
		exit(1);
	}
	char s[INET6_ADDRSTRLEN];
	inet_ntop(p->ai_family,(struct sockaddr *) get_in_addr(p->ai_addr), s, sizeof s);
	printf("Got message from %s:%d \n", s,ntohs(((struct sockaddr_in6*) p->ai_addr)->sin6_port));

	return numbytes;
}

//TODO: Remove?
uint64_t getPointerFromString(char* input) {
	uint64_t address = 0;
	memcpy(&input, &input, 64);
	
	// char message[100]={};
	// sprintf(message, "Received address: %" PRIx64 "\n", address);
	//print_debug("Received address: %" PRIx64 ".", address);

	uint64_t pointer = address;
	return pointer;
}

//TODO: Remove?
uint64_t getPointerFromIPv6Str(struct in6_addr addr) {
	char* pointer = (char *) malloc(12 * sizeof(unsigned char));
	char str[INET6_ADDRSTRLEN];

	unsigned int i;

	inet_ntop(AF_INET6, addr.s6_addr, str, INET6_ADDRSTRLEN);

	printf("String address: %s\n", str);

	int j = 0;
	for (i = strlen(str) - 14; i < strlen(str); i++) { // 14 b/c :
		if (str[i] != ':') {
			pointer[j] = str[i];
			j++;
		}
	}

	printf("Pointer: %s\n", pointer);

	return getPointerFromString(pointer);
}

uint64_t getPointerFromIPv6(struct in6_addr addr) {

	uint64_t temp = 0;
	char str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, addr.s6_addr, str, INET6_ADDRSTRLEN);
	printf("String address: %s\n", str);

	memcpy(&temp,addr.s6_addr+POINTER_SIZE,POINTER_SIZE);

	printf("Pointer: ");
	printNBytes((char*) &temp,POINTER_SIZE);

	return temp;
}


struct in6_addr getIPv6FromPointer(uint64_t pointer) {
	// Add the pointer

	struct in6_addr * newAddr = (struct in6_addr *) calloc(1, sizeof(struct in6_addr));
	memcpy(newAddr->s6_addr+IPV6_SIZE-POINTER_SIZE, (char *)pointer,POINTER_SIZE);
	memcpy(newAddr->s6_addr+6,SUBNET_ID,2);

	char s[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6,newAddr, s, sizeof s);
	print_debug("IPv6 Pointer %s",s);
	return *newAddr;
}
//TODO: Remove?
struct in6_addr getIPv6FromPointerStr(uint64_t pointer) {
	char* string_addr = (char*) calloc(IPV6_SIZE, sizeof(char));

	// Create the beginning of the address
	strcat(string_addr, GLOBAL_ID);
	strcat(string_addr, ":");
	strcat(string_addr, SUBNET_ID);
	strcat(string_addr, ":");// Pads the pointer

	// Add the pointer
	char* pointer_string = (char*) malloc(POINTER_SIZE * sizeof(char));
	
	//sprintf(pointer_string, "%" PRIx64, pointer);

	memcpy(&pointer_string, &pointer,POINTER_SIZE);
	printf("Pointer: %s\n", pointer_string);
	printf("Address so far: %s\n", string_addr);
	
	print_debug("Pointer length: %lu", strlen(pointer_string));
	print_debug("Address length: %lu", strlen(string_addr));

	unsigned int i;

	for (i = 0; i < strlen(pointer_string); i+=4) {
		char* substr = (char *) malloc(4 * sizeof(char));
		strcat(string_addr, ":");
		print_debug("Creating copy");
		strncpy(substr, pointer_string+i, 4);
		print_debug("Copy: %s", substr);
		print_debug("Performing concatenation");
		strcat(string_addr, substr);
		//strcat(string_addr, pointer_string[i]);
		//strcat(string_addr, pointer_string[i+1]);
		//strcat(string_addr, pointer_string[i+2]);
		//strcat(string_addr, pointer_string[i+3]);
	}

	print_debug("New address: %s", string_addr);

	struct in6_addr newAddr;

	if (inet_pton(AF_INET6, string_addr, &newAddr) == 1) {
		printf("SUCCESS: %s\n", string_addr);
		//successfully parsed string into "result"
	} else {
		printf("ERROR: not a valid address [%s]\n", string_addr);
		//failed, perhaps not a valid representation of IPv6?
	}

	return newAddr;
}