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

//Function that sends the file to the server. 
void send_file(int sockfd) { 
	char buffer[MAX]; //stores message to send to server                         
	
	FILE *fp=fopen("myconfig.json","r"); //opens file called myconfig.json, 'r' reads the file   
	
	while (fgets(buffer,MAX,fp) != NULL ) //puts the file into the buffer
		write(sockfd,buffer,strlen(buffer)); //writes to buffer which sends to server  

	fclose(fp);
	printf("File was sent successfully.\n");

}

//Function that parses the JSON file into JSON objects for use
void parse(int sockfd) {
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
    printf("Source_Port_Number_UDP: %s\n", json_object_get_string(Source_Port_Number_UDP));
    printf("Destination_Port_Number_TCP_Head: %s\n", json_object_get_string(Destination_Port_Number_TCP_Head));
    printf("Destination_Port_Number_TCP_Tail: %s\n", json_object_get_string(Destination_Port_Number_TCP_Tail));
    printf("Port_Number_TCP: %s\n", json_object_get_string(Port_Number_TCP));
    printf("Size_UDP_Payload: %s\n", json_object_get_string(Size_UDP_Payload));
    printf("Inter_Measurement_Time: %s\n", json_object_get_string(Inter_Measurement_Time));
    printf("Number_UDP_Packets: %d\n", json_object_get_int(Number_UDP_Packets));
    printf("TTL_UDP_Packets: %d\n", json_object_get_int(TTL_UDP_Packets));
}

int main(int argc, char *argv[]) { 
	int sockfd, connfd; 
	struct sockaddr_in serv_addr, cli; //create structure object of sockaddr_in for client and server

	// socket create and varification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); //creates a TCP socket
	if (sockfd == -1) { 
        printf("Socket Creation Failed.\n"); 
        exit(0); 
    } 
    else
        printf("Socket Successfully Created.\n"); 

	bzero(&serv_addr, sizeof(serv_addr));//zeroes out the server address

	serv_addr.sin_family = AF_INET; // specifies address family with IPv4 Protocol 
    serv_addr.sin_addr.s_addr = inet_addr("10.0.0.218"); //binds to IP Address
    serv_addr.sin_port = htons(8765); //binds to PORT

	// This connects the client socket to server socket 
	if (connect(sockfd, (SA*)&serv_addr, sizeof(serv_addr)) != 0) { 
		printf("Failed to connect to server.\n"); 
		exit(0); 
	} 
	else
		printf("Successfully connected to the server.\n"); 
	
	FILE *fp = fopen(argv[1], "r"); //opens file called in terminal which is myconfig.json

	//calling function to send file
	send_file(sockfd); 
	//calling function to parse JSON file
    parse(sockfd);
    //closes the socket after transfer
    close(sockfd); 
} 