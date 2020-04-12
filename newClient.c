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

void read_high_entropy_data(char * data, int len)
{
    FILE * f = fopen("/dev/random", "r");
    char temp;
    if(f == NULL)
    {
        return;
    }
    for(int i=0; i < len; i++)
    {
        temp = getc(f);
        printf("This is dev random: %d\n", temp);
        data[i] = temp;
    }

}

void read_low_entropy_data(char * data, int len)
{
    for(int i=0; i<len; i++)
    {
        data[i] = 0;
    }
}

int main()
{
    int sockfd;
    int length;
    char datagram[1024];
    char buffer[1024];
    struct sockaddr_in server_address;
    length = sizeof(datagram) / sizeof(char);
    

    read_high_entropy_data(datagram, length);

    read_low_entropy_data(datagram, length);

    printf("Creating Socket...\n");
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("socket creation failed\n"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("Socket created successfully!\n");
    }

    //bzero(server_address, &server_address);
    memset(&server_address, 0 , sizeof(server_address));

    server_address.sin_family = AF_INET;  //ipv4
    server_address.sin_port = htons(8765); //server port
    server_address.sin_addr.s_addr = inet_addr("192.168.1.26"); 

    int n, len;

    sendto(sockfd, datagram, sizeof(datagram), MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    printf("Message sent!\n");

    n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &server_address, &len); 
    buffer[n] = '\0'; 
    printf("Server : %s\n", buffer); 
  
    close(sockfd); 
    return 0;
}
