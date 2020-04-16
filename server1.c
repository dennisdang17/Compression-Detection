#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <json-c/json.h>
#define MAX 356
#define SA struct sockaddr 

void recvFile(int sockfd) 
{ 
    int i;
    char buff[MAX];     // to store message from client

    FILE *fp=fopen("myconfig.json","w");

    if( fp == NULL ){
        printf("Error IN Opening File ");
        return ;
    }

    while( read(sockfd,buff,MAX) > 0 )
        fprintf(fp,"%s",buff);
        fclose(fp);
} 

void parse(int sockfd) {
    char buffer[1024];
    struct json_object *parsed_json; //structure that holds parsed JSON
    //stores rest of fields of the JSON file
    struct json_object *Server_IP_Address;
    struct json_object *Source_Port_Number_UDP;
    struct json_object *Destination_Port_Number_TCP_Head;
    struct json_object *Destination_Port_Number_TCP_Tail;
    struct json_object *Port_Number_TCP;
    struct json_object *Size_UDP_Payload;
    struct json_object *Inter_Measurement_Time;
    struct json_object *Number_UDP_Packets;
    struct json_object *TTL_UDP_Packets;

    FILE *fp=fopen("myconfig.json","r");
    fread(buffer, 1024, 1, fp); //reads files and puts contents inside bufferD
    parsed_json = json_tokener_parse(buffer); //parse json file's contents and convert them into a json object

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
    printf("Source_Port_Number_UDP: %s\n", json_object_get_string(Source_Port_Number_UDP));
    printf("Destination_Port_Number_TCP_Head: %s\n", json_object_get_string(Destination_Port_Number_TCP_Head));
    printf("Destination_Port_Number_TCP_Tail: %s\n", json_object_get_string(Destination_Port_Number_TCP_Tail));
    printf("Port_Number_TCP: %s\n", json_object_get_string(Port_Number_TCP));
    printf("Size_UDP_Payload: %s\n", json_object_get_string(Size_UDP_Payload));
    printf("Inter_Measurement_Time: %s\n", json_object_get_string(Inter_Measurement_Time));
    printf("Number_UDP_Packets: %d\n", json_object_get_int(Number_UDP_Packets));
    printf("TTL_UDP_Packets: %d\n", json_object_get_int(TTL_UDP_Packets));

}



int main(int argc, char *argv[])  
{ 
    int sockfd, connfd, len;                // create socket file descriptor 
    struct sockaddr_in serv_addr, cli;      // create structure object of sockaddr_in for client and server

    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);           // creating a TCP socket ( SOCK_STREAM )

    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 

    // empty the 
    bzero(&serv_addr, sizeof(serv_addr)); 

    // assign IP, PORT 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("10.0.0.218");
    serv_addr.sin_port = htons(8765);

    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&serv_addr, sizeof(serv_addr))) != 0) { 
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
    FILE *fp = fopen(argv[1], "w");
    len = sizeof(cli); 

    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (SA*)&cli, &len);   // accepts connection from socket

    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 

        // Function for chatting between client and server 
        recvFile(connfd);
        parse(sockfd);
        // After transfer close the socket 
        close(sockfd); 

}