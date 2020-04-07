#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#define SA struct sockaddr
#define THRESH 100


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

// Communicates whether or not storage into the config_file struct was successful and if any info was missing
void storage_success(int sockfd, char * buffer, int len, struct config_file_data* config_data) 
{   
    char failure[5] = "ERROR";
	char success[8] = "SUCCESS";
    //Zero out the buffer to make sure it is empty to store the data
    for(int i=0; i<1000; i++)
    {
        buffer[i] = 0;
    }
    //Store info into buffer and then print it out
    recv(sockfd, buffer, 1000, 0); 
    printf("Here is all the info from client: %s\n", buffer);

    //Start storing the info into the config_data struct
    char* temp = buffer;
    while (*temp == ' ')
    {
        temp++;
    }

    //Sets the TCP header from the config file
    config_data->server_ip = strsep(&buffer, " ");
    config_data->src_prt_udp = strsep(&buffer, " ");
    config_data->dest_prt_udp = strsep(&buffer, " ");
    config_data->dest_prt_tcp_head = strsep(&buffer, " ");
    config_data->dest_prt_tcp_tail = strsep(&buffer, " ");
    config_data->tcp_port_num = strsep(&buffer, " ");
    config_data->payload_size = atoi(strsep(&buffer, " "));
    config_data->inter_time = atoi(strsep(&buffer, " "));
    config_data->num_of_packets = atoi(strsep(&buffer, " "));
    config_data->TTL = atoi(strsep(&buffer, " "));
	
    //If the struct has any missing data, return failure.
    if (config_data->server_ip == NULL || config_data->src_prt_udp == NULL ||
        config_data->dest_prt_udp == NULL || config_data->dest_prt_tcp_head == NULL ||
        config_data->dest_prt_tcp_tail == NULL || config_data->tcp_port_num == NULL ||
        config_data->payload_size < 1 || config_data->inter_time < 1 || config_data->num_of_packets < 1 ||
        config_data->TTL < 0)
    {
        printf("MISSING DATA FROM CONFIG FILE. CLOSING SOCKET\n");
        send(sockfd, failure, 8, 0);
    	close(sockfd);
    	return exit(1);
    }
    send(sockfd, success, 8, 0);
    return;
}

int accept_data(int socket_type, int* sockfd,struct sockaddr_in* server, struct sockaddr_in* client)
{
    
    int data_packet;
    int len;
    //Socket create and verification 
    *sockfd = socket(AF_INET, socket_type, 0); 
    if (*sockfd == -1)
    { 
        printf("Socket creation failed...\n"); 
        exit(0); 
    } 
    else
    {
        printf("Socket successfully created..\n");
    }

    //Zero out the server address
    bzero(server, sizeof(server));

    // assign IP, PORT 
    server->sin_family = AF_INET; 
    server->sin_addr.s_addr = htonl(INADDR_ANY); 
    server->sin_port = htons(8081); 

    // Binding newly created socket to given IP and verification 
    if ((bind(*sockfd, (SA*)server, sizeof(*server))) != 0)
    { 
        printf("Socket bind failed\n"); 
        exit(0); 
    } 
    else
    {
        printf("Socket successfully binded\n"); 
    }

    // Now server is ready to listen and verification 
    if ((listen(*sockfd, 5)) != 0)
    { 
        printf("Listen failed\n"); 
        exit(0); 
    } 
    else
    {
        printf("Server listening...\n");
    }

    // Accept the data packet from client and verification 
    len = sizeof(*client); 
    data_packet = accept(*sockfd, (SA*)client, &len); 
    if (data_packet < 0)
    { 
        printf("Server acccept failed\n"); 
        exit(0); 
    } 
    else
    {
        printf("Server acccepted the client\n");
    }
    return data_packet;
}


void recieve_train(struct config_file_data* config_data)
{
    int sockfd;
    int len;
    int end_of_buffer; ; 
    char buffer[1024]; 
    char* hello = "Hello from server"; 
    struct sockaddr_in server;
    struct sockaddr_in client; 
      
    //Create sockfd
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("Socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&server, 0, sizeof(server)); 
    memset(&client, 0, sizeof(client)); 
    // Filling server information 
    server.sin_family = AF_INET; // IPv4 
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(config_data->dest_prt_udp)); 
      
    // Bind the socket with the server address 
    if (bind(sockfd, (const struct sockaddr *) &server,sizeof(server)) < 0 )
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    len = sizeof(client);  //len is value/resuslt 
    end_of_buffer = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, ( struct sockaddr *) &client, &len); 
    buffer[end_of_buffer] = '\0';
    printf("Client : %s\n", buffer);
    sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &client, len); 
    printf("Hello message sent.\n"); 
}

  
//TODO
int main() 
{ 
    int sockfd;
    int connfd;
    int len;
    char* buffer1;
    struct sockaddr_in server;
    struct sockaddr_in cli; 
    struct sockaddr_in client;
    struct config_file_data* config_data; 
    char buffer2[1024];
    printf("I GOT HERE \n");
    //Pre-probing phase
    connfd = accept_data(SOCK_STREAM, &sockfd, &server, &cli);
    storage_success(connfd, buffer2, 1000, &config_data);
    printf("Ending connection\n");
    close(sockfd);

    sleep(20); 

    buffer1 =  calloc(config_data.payload_size, sizeof(char));  
    struct sockaddr_in  client;
    int fd;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    // Creating socket file descriptor 
    if (fd < 0 )
    { 
        perror("Socket creation failure\n"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&server, 0, sizeof(server)); 

      
    // Filling server information 
    server.sin_family = AF_INET; // IPv4 
    server.sin_addr.s_addr = htonl(INADDR_ANY); 
    server.sin_port = htons(atoi(config_data.dest_prt_udp)); 
      
    // Bind the socket with the server address 
    if (bind(fd, (const struct sockaddr *)&server, sizeof(server)) < 0 ) 
    { 
        perror("Bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("Bind successful\n");
    }
    

    //Probing Phase
    int n; 
    clock_t start_t, end_t;
    double total_t, low_entropy, high_entropy;
  
    len = sizeof(client);  //len is value/resuslt 

    start_t = clock();
    for (int i=0;i<config_data.num_of_packets;i++){ 
      
        n = recvfrom(fd, (char *)buffer, config_data.payload_sz,  
               MSG_WAITALL, ( struct sockaddr *) &client, 
               &len); 

    }
    end_t = clock();
    total_t = (((double)end_t) - ((double)start_t)) / ((double)CLOCKS_PER_SEC);
    low_entropy = total_t * 1000;
    printf("time : %f\n", low_entropy);

    sleep(tcp_info.in_time);

    start_t = clock();
    for (int i=0;i<config_data.num_of_packets;i++){        
        n = recvfrom(fd, (char *)buffer, config_data.payload_sz,  
                MSG_WAITALL, ( struct sockaddr *) &client, 
                &len);
    }
    end_t = clock();
    total_t = (((double)end_t) - ((double)start_t)) / ((double)CLOCKS_PER_SEC);
    high_entropy = total_t * 1000;
    printf("time 2 : %f\n", high_entropy);    


    close(fd);

    sleep(32);

    //final tcp connection

    struct sockaddr_in serv_end, cliaddr_end;
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1)
    { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
    {
        printf("Socket successfully created..\n");
    }
    bzero(&serv_end, sizeof(serv_end)); 
  
    // assign IP, PORT 
    serv_end.sin_family = AF_INET; 
    serv_end.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_end.sin_port = htons(8082); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&serv_end, sizeof(serv_end))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    len = sizeof(cliaddr_end); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (SA*)&cliaddr_end, &len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 

    char compression[50];

    // now we will do the check: If (∆tH − ∆tL) is bigger than our threshold (100 ms) then we have compression
    if ((high_entropy - low_entropy) > THRESH)
    {
        strcpy(compression, "COMPRESSION DETECTED");
    }
    else
    {
        strcpy(compression, "COMPRESSION NOT DETECTED");

    }

    bzero(buff, sizeof(buff));

    // read the message from client and copy it in buffer 
    recv(sockfd, buff, 1000, 0);

    send(connfd, compression, 50, 0);

    free(buffer);
    close(sockfd);

} 