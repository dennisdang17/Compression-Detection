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

#define PAYLOAD_SIZE 1000

//Filling the payload with data
void read_high_entropy_data(char * data, int len)
{
    FILE * f = fopen("/dev/urandom", "r");
    char temp;
    if(f == NULL)
    {
        return;
    }
    for(int i=0; i < len; i++)
    {
        temp = getc(f);
        data[i] = temp;
    }
}

//Filling payload with low entropy data
void read_low_entropy_data(char * data, int len)
{
    for(int i=0; i<len; i++) 
    {
        data[i] = 0;
    }
}

void set_packet_id(char * data, int index)
{
    data[0] = index;
}

int main()
{
    int sockfd, length, DF;
    char datagram[PAYLOAD_SIZE];
    char buffer[25];
    struct sockaddr_in server_address;
    length = sizeof(datagram) / sizeof(char);
    

    //preProbing.


    //creating socket
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


    //set server struct header
    memset(&server_address, 0 , sizeof(server_address));
    server_address.sin_family = AF_INET;  //ipv4
    server_address.sin_port = htons(8765); //server port
    server_address.sin_addr.s_addr = inet_addr("192.168.1.30"); //Change to configfile.serverIP
    
    
    //Don't fragment bit
    DF = IP_PMTUDISC_DO; //make val equal to dont fragment
    printf("Setting DON'T FRAGMENT bit...\n");
    if(setsockopt( sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &DF, sizeof(DF)) < 0)
    {
        printf("Unable to set DON'T FRAGMENT bit\n");
    }
    else
    {
        printf("DON'T FRAGMENT bit set correctly!\n");
    }


    //Probing phase. Sending packet of data


    //send low entropy now
    read_low_entropy_data(datagram, length);
    for(int i=0; i < 50; i++)//chagne to payload size
    {
        //set_packet_id(datagram, i);
        sendto(sockfd, datagram, sizeof(datagram), MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    }
    printf("Low entropy sent!!\n");


    printf("Sleeping...\n");
    sleep(15); //replace with inter time
    

    //send high entropy now
    read_high_entropy_data(datagram, length);
    for(int i=0; i < 50; i++) //change to payload size
    {
        //set_packet_id(datagram, i);
        sendto(sockfd, datagram, sizeof(datagram), MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    }
    printf("High entropy sent!!\n");


    //Post Probing

    //receive response
    unsigned int len;
    recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &server_address, &len);  
    printf("Server : %s\n", buffer); 
    close(sockfd); 
    return 0;
}
