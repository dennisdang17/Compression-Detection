#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()
#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, IPPROTO_UDP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/udp.h>      // struct udphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <errno.h>            // errno, perror()

// Define some constants.
#define IP4_HDRLEN 20         // IPv4 header length
#define UDP_HDRLEN  8         // UDP header length, excludes data

int main()
{
    int sockfd;
    char datagram[1024];
    char buffer[1024];
    for (int i = 0; i < 1024; i++)
    {
        datagram[i] = 0;
    }
    struct socketaddr_in server_address;
    
    printf("Creating Socket...");
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("Socket created successfully!");
    }

    bzero(server_address, &server_address);

    server_address.sin_family = AF_INET;  //ipv4
    server_address.sin_port = htons(8765); //server port
    server_address.sin_addr.s_addr = INADDR_ANY; 

    int n, len;

    sendto(sockfd, (const char *), sizeof(datagram), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
    printf("Message sent!");

    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
    buffer[n] = '\0'; 
    printf("Server : %s\n", buffer); 
  
    close(sockfd); 
    return 0;
}