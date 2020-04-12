#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h> 
#include <string.h>
#include <arpa/inet.h>
//#include "read_json.h"
#define SA struct sockaddr 
#define PORT 8080 
#define MAXLINE 1024 

struct config_file_data
{
    char * server_ip;
    char * src_prt_udp;
    char * dest_prt_udp;
    char * dest_prt_tcp_head;
    char * dest_prt_tcp_tail;
    char * tcp_port_num;
    unsigned short int payload_size;
    int inter_time;
    int num_of_packets;
    int TTL;
};

void packet_id_setup (char* bin, unsigned int val) {
    unsigned int val2 = val;
    for(int i = 15; i >= 0 ;i--)
    {
        bin[i] = (copy_of_value & 0b1) +'0';
        copy_of_value >>= 1;
    }
}

void high_entropy_packet (uint8_t * data, int len) {
    FILE* f = NULL;
    char temp;
    f =  fopen("/dev/random", "r");
    if(f == NULL) {
        return EXIT_FAILURE;
    }
    for (int i = 0; i < len; i++) {
        temp = getc(f); //grabs a char and then tokenizes it.
        int num = atoi(&temp);
        if (num > 1) //if the number is greater than 1, just make it 1 
        {
            num = 1;
        }
        data[i] = num;
    }
}

void make_packet (struct config_file_data packet_info, int socket_type, int* sockfd,struct sockaddr_in* clientaddr) {
    *sockfd = socket(AF_INET, socket_type, 0); 
    if (*sockfd == -1) { 
        printf("Failed to create socket\n"); 
        exit(0); 
    } 
    else
        printf("Succesfully Created Socket\n");
        bzero(clientaddr, sizeof(*clientaddr)); 
    while(*packet_info.server_ip == ' ')
    {
        packet_info.server_ip++;
    }
    clientaddr->sin_family = AF_INET; 
    clientaddr->sin_addr.s_addr = inet_addr(packet_info.server_ip);
    clientaddr->sin_port = htons(atoi(packet_info.prt_tcp)); 
}

uint8_t * allocate_ustrmem(int len) {
  void *tmp;
  if (len <= 0) {
        fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
        exit (EXIT_FAILURE);
    }
        tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
        if (tmp != NULL) {
        memset (tmp, 0, len * sizeof (uint8_t));
        return (tmp);
    } else {
        fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
        exit (EXIT_FAILURE);
    }
}

//sending packet train
void send_train(struct config_file_data* packet_info)
{
    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = "Hello from client"; 
    struct sockaddr_in  
    servaddr; 

    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET;
    printf("dest prt : %d\n", atoi(packet_info->dest_prt_udp)); 
    servaddr.sin_port = htons(atoi(packet_info->dest_prt_udp)); 
    servaddr.sin_addr.s_addr = inet_addr(packet_info->server_ip); 
      
    int n, len; 
      
    sendto(sockfd, (const char *)hello, strlen(hello), 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr)); 
    printf("Hello message sent.\n"); 
          
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &len); 
    buffer[n] = '\0'; 
    printf("Server : %s\n", buffer); 
    close(sockfd); 
}
  
int main() 
{ 
    char binary[16];
    unsigned int packet_id = 0;
    int sockfd, connfd, val, clientlen; 
    struct sockaddr_in clientaddr, cli;
    //struct json packet_info;
    uint8_t *data;
    
    char buff[1000] = {0};
    //read_json(&packet_info, "myconfig.json", buff); 
    make_packet(packet_id, SOCK_STREAM, &sockfd, &clientaddr);
    if (connect(sockfd, (SA*)&clientaddr, sizeof(clientaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
    printf("connected to the server..\n"); 
    send(sockfd, buff, (strlen(buff)+1), 0);
    char new[8];
    bzero(new, 8);
    recv(sockfd, new, 8,0);

    if (strncmp(new, "SUCCESS", 7) != 0) {
        printf("FAILED PREPROBING PHASE EXITING...");
        exit(0);
    }
    else {
        printf("ended conn\n");
    }
    close(sockfd); 

    /**
        Probing phase [send in UDP packet trains of high and low entropy data each of quantity 6000] 
    */
    sleep (25);
    //send_packet_train(&packet_info);
    struct sockaddr_in addr, srcaddr;
    int fd;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(packet_id.server_ip);
    addr.sin_port = htons(atoi(packet_id.dest_prt_udp));

    memset(&srcaddr, 0, sizeof(srcaddr));
    srcaddr.sin_family = AF_INET;
    srcaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srcaddr.sin_port = htons(atoi(packet_id.src_prt_udp));
    val=IP_PMTUDISC_DO;

    if (setsockopt(fd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val))<0) {
        printf("unable to set DONT_FRAGMENT bit...\n"); 
        exit(0); 
    }
    else
        printf("DONT_FRAGMENT bit set successfully..\n");

    if (bind(fd, (struct sockaddr *) &srcaddr, sizeof(srcaddr)) < 0) {
        perror("bind");
        exit(1);
    }

    if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
    { 
        printf("\n Error : Connect Failed \n"); 
        exit(0); 
    } 

    //let's setup UDP low entropy payload

    data = allocate_ustrmem(packet_info.payload_sz);
    clientlen = sizeof(addr);

    for (int i=0;i<packet_info.num_of_packets;i++){
        packet_id_setup(data, packet_id++);
        if(sendto(fd,data,packet_info.payload_sz,MSG_CONFIRM,(struct sockaddr *) &addr,clientlen)<=0){
            continue;
        }  
    }

    sleep(packet_info.in_time);
    bzero(data, packet_info.payload_sz);
    high_entropy_packet(&data[16], packet_info.payload_sz-16);
    for (int i=0;i<packet_info.num_of_packets;i++){
        packet_id_setup(data, packet_id++);
        if(sendto(fd,data,packet_info.payload_sz,MSG_CONFIRM,(struct sockaddr *) &addr,clientlen)<=0) {
            continue;
        }  
    }
    printf("All packages sent closing connection\n");
    close(fd); 
    sleep(34);
    struct sockaddr_in servaddr; 
  
    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    
    //Post-probing phase [send out compression information through TCP connection]

    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(packet_info.server_ip); 
    servaddr.sin_port = htons(8082); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
    printf("connected to the server..\n"); 
    bzero(buff, 1000); 
    send(sockfd, buff, 1000, 0); 
    bzero(buff, 1000); 
    recv(sockfd, buff, 50, 0);
    printf("%s\n",buff); 
} 

