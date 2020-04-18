#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>          
#include <string.h>           
#include <netdb.h>           
#include <sys/types.h>        
#include <sys/socket.h>       
#include <netinet/in.h>       
#include <arpa/inet.h>
#include <errno.h> //error reports
#include <time.h> //clock
#include <ctype.h>
#include <json-c/json.h> //for json parsing

#define BUF_SIZE 2000
#define THRESHOLD 100

void receive_file(int sockfd)
{
    char buffer[BUF_SIZE]; // stores message from client 

    FILE *fp=fopen("myconfig.json","w"); //creates a file named myconfig.json in the program directory, 'w' opens for writing purposes
    if( fp == NULL )
    {
        printf("There is an error in opening the file.");
        return exit(EXIT_FAILURE);
    }

    while(read(sockfd, buffer, BUF_SIZE) > 0) //reads the file into the buffer
    {
        fprintf(fp,"%s",buffer);
    }
    fclose(fp);
} 

int main(int argc, char * argv[])
{
    int TCPsockfd, connfd;
    unsigned int len;
    char buffer[BUF_SIZE], message[25];
    struct sockaddr_in server_address, client_address;
    struct json_object *parsed_json, *Server_IP_Address, *Source_Port_Number_UDP,
    *Destination_Port_Number_TCP_Head, *Destination_Port_Number_TCP_Tail, *Port_Number_TCP, 
    *Size_UDP_Payload, *Inter_Measurement_Time, *Number_UDP_Packets, *TTL_UDP_Packets;

    //tcp connection
    if((TCPsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket Creation Failed for TCP\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket created successfully!\n");
    }

    //Zeroing out and filling in the client and server headers
    memset(&server_address, 0 , sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));
    server_address.sin_family = AF_INET; // IPv4 
    server_address.sin_addr.s_addr = inet_addr("192.168.1.28"); //hard coded ip
    server_address.sin_port = htons(8765); //port numbers

    if ((bind(TCPsockfd, (struct sockaddr *) &server_address, sizeof(server_address))) != 0)
    { 
        printf("The socket bind failed\n"); 
        exit(0); 
    } 
    else
    {
        printf("The socket was successfully binded.\n"); 
    }

    if ((listen(TCPsockfd, 5)) != 0) //server is ready to listen
    { 
        printf("Failed to listen\n"); 
        exit(0); 
    } 
    else
    {
        printf("Server is listening\n"); 
    }
    
    len = sizeof(client_address); 
    if ((connfd = accept(TCPsockfd, (struct sockaddr *) &client_address, &len)) < 0)
    { 
        printf("Server failed to accept client.\n"); 
        exit(0); 
    } 
    else
    {
        printf("Server successfully accepted the client.\n"); 
    }
    //calling function to receive file
    receive_file(connfd);

    //file parsing happens here
    FILE *fp = fopen(argv[1],"r"); //opens the file myconfig.json
    fread(buffer, 1024, 1, fp); //reads files and puts contents inside buffer
    parsed_json = json_tokener_parse(buffer);

    json_object_object_get_ex(parsed_json, "Server_IP_Address", &Server_IP_Address);
    json_object_object_get_ex(parsed_json, "Source_Port_Number_UDP", &Source_Port_Number_UDP);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Head", &Destination_Port_Number_TCP_Head);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Tail", &Destination_Port_Number_TCP_Tail);
    json_object_object_get_ex(parsed_json, "Port_Number_TCP", &Port_Number_TCP);
    json_object_object_get_ex(parsed_json, "Size_UDP_Payload", &Size_UDP_Payload);
    json_object_object_get_ex(parsed_json, "Inter_Measurement_Time", &Inter_Measurement_Time);
    json_object_object_get_ex(parsed_json, "Number_UDP_Packets", &Number_UDP_Packets);
    json_object_object_get_ex(parsed_json, "TTL_UDP_Packets", &TTL_UDP_Packets);

    printf("Server_IP_Address: %s\n", json_object_get_string(Server_IP_Address));
    printf("Source_Port_Number_UDP: %d\n", json_object_get_int(Source_Port_Number_UDP));
    printf("Destination_Port_Number_TCP_Head: %s\n", json_object_get_string(Destination_Port_Number_TCP_Head));
    printf("Destination_Port_Number_TCP_Tail: %s\n", json_object_get_string(Destination_Port_Number_TCP_Tail));
    printf("Port_Number_TCP: %d\n", json_object_get_int(Port_Number_TCP));
    printf("Size_UDP_Payload: %d\n", json_object_get_int(Size_UDP_Payload));
    printf("Inter_Measurement_Time: %d\n", json_object_get_int(Inter_Measurement_Time));
    printf("Number_UDP_Packets: %d\n", json_object_get_int(Number_UDP_Packets));
    printf("TTL_UDP_Packets: %d\n", json_object_get_int(TTL_UDP_Packets));

    close(TCPsockfd);
    close(connfd);

    //PROBING PHASE

    int UDPsockfd;
    memset(&buffer, 0 , BUF_SIZE);
    
    //Creation of socket
    if ((UDPsockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("Socket successfully created!\n");
    }

    //bind server ip to socket
    if (bind(UDPsockfd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("Bind successful\n");
    }

    //Start of probing phase
    socklen_t length = sizeof(struct sockaddr_storage);
    clock_t start_time, end_time;
    double total_time, low_entropy_time, high_entropy_time;

    //receive low entropy data
    printf("Receiving low entropy..\n");
    start_time = clock();
    for(int i = 0; i < json_object_get_int(Number_UDP_Packets); i++)//replace 10 with the number of packets
    {
        recvfrom(UDPsockfd, buffer, BUF_SIZE, 0, ( struct sockaddr *) &client_address, &length);
        int packet_id = (int)(((unsigned)buffer[1] << 8) | buffer[0]);
        printf("Retrieved packet. Packet Number: %d\n", packet_id);
    }
    end_time = clock();
    total_time  = (((double)end_time) - ((double)start_time)) / ((double)CLOCKS_PER_SEC);
    low_entropy_time = total_time * 1000;
    printf("Low Entropy Time: %f\n", low_entropy_time);


    printf("Sleeping...\n");
    sleep(json_object_get_int(Inter_Measurement_Time)); //sleep inter time


    //receive high entropy data
    printf("Receiving high entropy..\n");
    memset(&buffer, 0, BUF_SIZE);
    start_time = clock();
    for(int i = 0; i < json_object_get_int(Number_UDP_Packets); i++)//replace 10 with the number of packets
    {
        recvfrom(UDPsockfd, buffer, BUF_SIZE, 0, ( struct sockaddr *) &client_address, &length);
        int packet_id = (int)(((unsigned)buffer[1] << 8) | buffer[0]);
        printf("Retrieved Packet Number: %d\n", packet_id);
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
    
    //tcp then send message
    
    sendto(UDPsockfd, message, strlen(message), 0, (const struct sockaddr *) &client_address, sizeof(client_address));
    printf("Message sent\n");
    return 0;
    

}
