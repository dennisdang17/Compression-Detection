#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()
#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t,
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, IPPROTO_UDP, INET_ADDRSTRLEN
#include <arpa/inet.h>        // inet_pton() and inet_ntop()

#define BUF_SIZE 1024
int main()
{
    int sockfd;
    char datagram[1024];
    char buffer[1024];
    char * message = "Package received! Thank you!";
    for (int i = 0; i < 1024; i++)
    {
        datagram[i] = 0;
    }
    struct sockaddr_in server_address, client_address;
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("Socket successfully created!\n");
    }

    //bzero(client_address, &client_address);
    //bzero(server_address, &server_address);
    memset(&server_address, 0 , sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));
    // Filling server information 
    server_address.sin_family = AF_INET; // IPv4 
    server_address.sin_addr.s_addr = inet_addr("192.168.1.26"); 
    server_address.sin_port = htons(8765);
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr("192.168.1.29");
    client_address.sin_port = htons(8765);

    char test[INET_ADDRSTRLEN];
    inet_ntop(server_address.sin_family, &server_address.sin_addr.s_addr, test, INET6_ADDRSTRLEN);
    printf("IP ADDRESS: %s\n", test);
    int rv;

    if (rv = bind(sockfd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("Bind successful\n");
    }

    int a = 0;
    socklen_t len = sizeof(struct sockaddr_storage); 

    printf("Listening ...\n");

    int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, ( struct sockaddr *) &client_address, &len);
    printf("Client's Data is : ");
    for(int i=0; i<BUF_SIZE; i++)
    {
       printf("%d ", buffer[i]);
    }
    sendto(sockfd, message, strlen(message), 0, &client_address, sizeof(client_address));
    printf("Message sent\n");
    return 0;
    

}
