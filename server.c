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
#define BUF_SIZE 2000
#define THRESHOLD 100


int main()
{
    int sockfd;
    char buffer[BUF_SIZE];
    char message[25];
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

    //bind server ip to socket
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
    clock_t start_time, end_time;
    double total_time, low_entropy_time, high_entropy_time;
    
    printf("Listening ...\n");

    //receive low entropy data
    printf("Receiving low entropy..\n");
    start_time = clock();
    for(int i = 0; i < 50; i++)//replace 10 with the number of packets
    {
        recvfrom(sockfd, buffer, BUF_SIZE, 0, ( struct sockaddr *) &client_address, &len);
    }
    end_time = clock();
    total_time  = (((double)end_time) - ((double)start_time)) / ((double)CLOCKS_PER_SEC);
    low_entropy_time = total_time * 1000;
    printf("Low Entropy Time: %f\n", low_entropy_time);


    printf("Sleeping...\n");
    sleep(15); //replace with inter time


    //receive high entropy data
    printf("Receiving high entropy..\n");
    memset(&buffer, 0, BUF_SIZE);
    start_time = clock();
    for(int i = 0; i < 50; i++)//replace 10 with the number of packets
    {
        recvfrom(sockfd, buffer, BUF_SIZE, 0, ( struct sockaddr *) &client_address, &len);
    }
    end_time = clock();
    total_time  = (((double)end_time) - ((double)start_time)) / ((double)CLOCKS_PER_SEC);
    high_entropy_time = total_time * 1000;
    printf("High Entropy Time: %f\n", high_entropy_time);


    //Calculate the time and then send back the data
    if((high_entropy_time - low_entropy_time) > THRESHOLD)
    {
        strcpy(message, "COMPRESSION DETECTED\0");
    }
    else
    {
        strcpy(message, "NO COMPRESSION DETECTED\0");
    }
    
    

    //Post probing TCP CONNECTION
    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *) &client_address, sizeof(client_address));
    printf("Message sent\n");
    return 0;
    

}
