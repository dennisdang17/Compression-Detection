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

int main()
{
    int sockfd;
    char datagram[1024];
    char buffer[1024];
    char * message = "Hello nigga ty.";
    for (int i = 0; i < 1024; i++)
    {
        datagram[i] = 0;
    }
    struct socketaddr_in server_address, client_address;
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("Socket successfully created!\n");
    }

    bzero(client_address, &client_address);
    bzero(server_address, &server_address)
      
    // Filling server information 
    server_address.sin_family    = AF_INET; // IPv4 
    server_address.sin_addr.s_addr = INADDR_ANY; 
    server_address.sin_port = htons(8765);

    if (bind(sockfd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("Bind successful\n");
    }

    int len, n;
    int a = 0;
    len = sizeof(client_address);

    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, ( struct sockaddr *) &client_address, &len);
    buffer[n] = '\0'
    printf("Client's Data is : ");
    while(buffer[a] != '\0')
    {
        printf("%d",buffer[a])
    }

    sendto(sockfd, message, strlen(message), MSG_CONFIRM, &client_address, len);
    printf("Message sent\n");
    return 0;
    

}