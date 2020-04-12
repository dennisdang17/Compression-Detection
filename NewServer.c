#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>          
#include <string.h>           
#include <netdb.h>           
#include <sys/types.h>        
#include <sys/socket.h>       
#include <netinet/in.h>       
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#define BUF_SIZE 1024


int main()
{
    int sockfd;
    char buffer[BUF_SIZE];
    char * message = "Package received! Thank you!";
    struct sockaddr_in server_address, client_address;

    //Creation of socket
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("Socket successfully created!\n");
    }

    //zero out the client and server structs
    memset(&server_address, 0 , sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    // Filling server and client information.
    server_address.sin_family = AF_INET; // IPv4 
    server_address.sin_addr.s_addr = inet_addr("192.168.1.30"); //hard coded ip
    server_address.sin_port = htons(8765); //port numbers

    client_address.sin_family = AF_INET; //IPV4
    client_address.sin_addr.s_addr = inet_addr("192.168.1.26"); //hard coded ip
    client_address.sin_port = htons(8765); //port numbers

    
    //bind ip to socket
    if (bind(sockfd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("Bind successful\n");
    }


    //Start of probing phase
    socklen_t len = sizeof(struct sockaddr_storage);
    //clock_t start_time, end_time;
    
    printf("Listening ...\n");
    //start_time = clock();
    
    //in a forloop
    recvfrom(sockfd, buffer, BUF_SIZE, 0, ( struct sockaddr *) &client_address, &len);


    printf("Client's Data is : ");
    for(int i=0; i<BUF_SIZE; i++)
    {
       printf("%d ", buffer[i]);
    }
    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *) &client_address, sizeof(client_address));
    printf("Message sent\n");
    return 0;
    

}
