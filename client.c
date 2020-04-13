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

//Filling the payload with data

/*void read_high_entropy_data(char * data, int len)
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

}*/

void test_high_entropy_data(char* data, int len)
{
    srand(time(NULL));
    for(int i=0; i<len; i++)
    {
	data[i] = rand() %127;
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

    
    memset(&server_address, 0 , sizeof(server_address));

    //set server struct header
    server_address.sin_family = AF_INET;  //ipv4
    server_address.sin_port = htons(8765); //server port
    server_address.sin_addr.s_addr = inet_addr("192.168.1.30"); 
    
    //Probing phase. Sending packet of data

    //Making data packets

    //send low entropy now
    read_low_entropy_data(datagram, length);
    for(int i=0; i < 1000; i++)//change 10 iwth # of packets
    {
        sendto(sockfd, datagram, sizeof(datagram), MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    }
    printf("Low entropy sent!!\n");

    printf("Sleeping...\n");
    sleep(15);
    
    //send high entropy now
    //read_high_entropy_data(datagram, length);
    for(int i=0; i < 1000; i++)
    {
        test_high_entropy_data(datagram, length);
        sendto(sockfd, datagram, sizeof(datagram), MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    }
    sendto(sockfd, datagram, sizeof(datagram), MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    printf("High entropy sent!!\n");

    //receive response
    int end_of_buffer;
    unsigned int len;
    end_of_buffer = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *) &server_address, &len); 
    buffer[end_of_buffer] = '\0'; 
    printf("Server : %s\n", buffer); 
  
    close(sockfd); 
    return 0;
}
