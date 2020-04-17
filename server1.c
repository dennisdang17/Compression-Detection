#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <json-c/json.h> //header for JSON Parser
#define MAX 356
#define SA struct sockaddr 

//Function that receives the file from the client.
void receive_file(int sockfd) {
    char buffer[MAX]; // stores message from client 

    FILE *fp=fopen("myconfig.json","w"); //creates a file named myconfig.json in the program directory, 'w' opens for writing purposes
    if( fp == NULL ){
        printf("There is an error in opening the file.");
        return ;
    }

    while(read(sockfd,buffer,MAX) > 0) //reads the file into the buffer
        fprintf(fp,"%s",buffer); 
        fclose(fp);
} 

//Function that parses the JSON file into JSON objects for use
int main(int argc, char *argv[]) { 
    int sockfd, connfd, len; 
    struct sockaddr_in serv_addr, cli; //create structure object of sockaddr_in for client and server

    // This is where the sockets are created.
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //creates a TCP socket

    if (sockfd == -1) { 
        printf("Socket Creation Failed.\n"); 
        exit(0); 
    } 
    else
        printf("Socket Successfully Created.\n"); 

    bzero(&serv_addr, sizeof(serv_addr)); //zeroes out server address

    serv_addr.sin_family = AF_INET; // specifies address family with IPv4 Protocol 
    serv_addr.sin_addr.s_addr = inet_addr("10.0.0.218"); //binds to IP Address
    serv_addr.sin_port = htons(8765); //binds to PORT

    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&serv_addr, sizeof(serv_addr))) != 0) { 
        printf("The socket bind failed\n"); 
        exit(0); 
    } 
    else
    printf("The socket was successfully binded.\n"); 

    if ((listen(sockfd, 5)) != 0) { //server is ready to listen
        printf("Failed to listen\n"); 
        exit(0); 
    } 
    else
        printf("Server is listening\n"); 
    
    FILE *fp = fopen(argv[1], "w");//opens file called in terminal which is myconfig.json
    
    len = sizeof(cli); 

    connfd = accept(sockfd, (SA*)&cli, &len); //accepts connection from socket

    if (connfd < 0) { 
        printf("Server failed to accept client.\n"); 
        exit(0); 
    } 
    else
        printf("Server successfully accepted the client.\n"); 

    //calling function to receive file
    receive_file(connfd);

    //This is where the JSON file is parsed.
    char buffer[1024];
    struct json_object *parsed_json; //structure that holds parsed JSON
    //structs that store the rest of fields of the JSON file
    struct json_object *Server_IP_Address;
    struct json_object *Source_Port_Number_UDP;
    struct json_object *Destination_Port_Number_TCP_Head;
    struct json_object *Destination_Port_Number_TCP_Tail;
    struct json_object *Port_Number_TCP;
    struct json_object *Size_UDP_Payload;
    struct json_object *Inter_Measurement_Time;
    struct json_object *Number_UDP_Packets;
    struct json_object *TTL_UDP_Packets;

    FILE *fp=fopen("myconfig.json","r"); //opens the file myconfig.json
    fread(buffer, 1024, 1, fp); //reads files and puts contents inside buffer
    parsed_json = json_tokener_parse(buffer); //parse JSON file's contents and converts them into a JSON object

    //this function gets the value of the key in the JSON objects 
    json_object_object_get_ex(parsed_json, "Server_IP_Address", &Server_IP_Address);
    json_object_object_get_ex(parsed_json, "Source_Port_Number_UDP", &Source_Port_Number_UDP);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Head", &Destination_Port_Number_TCP_Head);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Tail", &Destination_Port_Number_TCP_Tail);
    json_object_object_get_ex(parsed_json, "Port_Number_TCP", &Port_Number_TCP);
    json_object_object_get_ex(parsed_json, "Size_UDP_Payload", &Size_UDP_Payload);
    json_object_object_get_ex(parsed_json, "Inter_Measurement_Time", &Inter_Measurement_Time);
    json_object_object_get_ex(parsed_json, "Number_UDP_Packets", &Number_UDP_Packets);
    json_object_object_get_ex(parsed_json, "TTL_UDP_Packets", &TTL_UDP_Packets);

    //print function that tests wether or not the parsing is successful
    printf("Server_IP_Address: %s\n", json_object_get_string(Server_IP_Address));
    printf("Source_Port_Number_UDP: %d\n", json_object_get_int(Source_Port_Number_UDP));
    printf("Destination_Port_Number_TCP_Head: %s\n", json_object_get_string(Destination_Port_Number_TCP_Head));
    printf("Destination_Port_Number_TCP_Tail: %s\n", json_object_get_string(Destination_Port_Number_TCP_Tail));
    printf("Port_Number_TCP: %d\n", json_object_get_int(Port_Number_TCP));
    printf("Size_UDP_Payload: %d\n", json_object_get_int(Size_UDP_Payload));
    printf("Inter_Measurement_Time: %d\n", json_object_get_int(Inter_Measurement_Time));
    printf("Number_UDP_Packets: %d\n", json_object_get_int(Number_UDP_Packets));
    printf("TTL_UDP_Packets: %d\n", json_object_get_int(TTL_UDP_Packets));

    //closes the socket after transfer
    close(sockfd); 
    
}