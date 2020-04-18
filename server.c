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

    FILE *fp = fopen("myconfig.json","w"); //creates a file named myconfig.json in the program directory, 'w' opens for writing purposes
    if( fp == NULL )
    {
        printf("There is an error in opening the file.");
        return exit(EXIT_FAILURE);
    }

    while(read(sockfd, buffer, BUF_SIZE) > 0) //reads the file into the buffer
    {
        fprintf(fp,"%s", buffer);
    }
    printf("Config File received!\n");
    fclose(fp);
} 

int main(int argc, char * argv[])
{
    int sockfd, connfd, i, packet_id;
    unsigned int len;
    char buffer[BUF_SIZE], message[25];
    FILE * fp;
    clock_t start_time, end_time;
    double total_time, low_entropy_time, high_entropy_time;
    struct sockaddr_in server_address, client_address;
    struct json_object *parsed_json, *Server_IP_Address, *Source_Port_Number_UDP, *Destination_Port_Number_UDP,
    *Destination_Port_Number_TCP_Head, *Destination_Port_Number_TCP_Tail, *Port_Number_TCP, 
    *Size_UDP_Payload, *Inter_Measurement_Time, *Number_UDP_Packets, *TTL_UDP_Packets;

    if(argv[1] == NULL) //check if usage is correct
    {
        printf("ERROR!\nUsage is ./server 'my_config_file'\n");
        return EXIT_FAILURE;
    }
    

    //Pre-Probing Phase TCP


    //Creation of TCP socket
    printf("Creating Socket...\n");
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("TCP Socket Creation Failed\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("TCP Socket Creation Successful\n");
    }

    //Fill in IP header
    memset(&server_address, 0 , sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));
    server_address.sin_family = AF_INET; // IPv4 
    server_address.sin_addr.s_addr = inet_addr("192.168.1.30"); //hard coded ip
    server_address.sin_port = htons(9999); //Port number

    //Bind TCP socket to port
    printf("Binding...\n");
    if ((bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address))) != 0)
    { 
        printf("TCP Socket Bind Failed\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("TCP Socket Bind Successful\n"); 
    }

    //Listen to receive connections at port
    printf("Listening...\n");
    if ((listen(sockfd, 5)) != 0)
    { 
        printf("Listening Failed\n"); 
        exit(EXIT_FAILURE); 
    } 
    
    //Accept the connection
    len = sizeof(client_address); 
    if ((connfd = accept(sockfd, (struct sockaddr *) &client_address, &len)) < 0)
    { 
        printf("TCP Connection Failed\n"); 
        exit(0); 
    } 
    else
    {
        printf("TCP Connection Established\n"); 
    }

    //calling function to receive file from connection
    receive_file(connfd);

    //file parsing happens here
    fp = fopen(argv[1],"r"); //opens the file myconfig.json
    fread(buffer, BUF_SIZE, 1, fp); //reads files and puts contents inside buffer
    parsed_json = json_tokener_parse(buffer);

    //Storing the data into the correct variables
    json_object_object_get_ex(parsed_json, "Server_IP_Address", &Server_IP_Address);
    json_object_object_get_ex(parsed_json, "Source_Port_Number_UDP", &Source_Port_Number_UDP);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_UDP", &Destination_Port_Number_UDP);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Head", &Destination_Port_Number_TCP_Head);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Tail", &Destination_Port_Number_TCP_Tail);
    json_object_object_get_ex(parsed_json, "Port_Number_TCP", &Port_Number_TCP);
    json_object_object_get_ex(parsed_json, "Size_UDP_Payload", &Size_UDP_Payload);
    json_object_object_get_ex(parsed_json, "Inter_Measurement_Time", &Inter_Measurement_Time);
    json_object_object_get_ex(parsed_json, "Number_UDP_Packets", &Number_UDP_Packets);
    json_object_object_get_ex(parsed_json, "TTL_UDP_Packets", &TTL_UDP_Packets);

    close(sockfd);
    close(connfd);


    //PROBING PHASE//


    //Set up the server address for the udp header
    int UDPbuffer[json_object_get_int(Size_UDP_Payload)+2];
    memset(&UDPbuffer, 0 , sizeof(UDPbuffer));
    server_address.sin_addr.s_addr = inet_addr(json_object_get_string(Server_IP_Address)); //hard coded ip
    server_address.sin_port = htons(json_object_get_int(Destination_Port_Number_UDP)); //port number

    //Creation of UDP socket
    printf("Creating Socket...\n");
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("UDP Socket Creation Failed"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("UDP Socket Creation Successful\n");
    }

    //Bind the socket to the udp port
    printf("Binding...\n");
    if (bind(sockfd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0 ) 
    { 
        perror("UDP Socket Bind Failed"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("UDP Socket Bind Successful\n");
    }

    //Receive low entropy data
    printf("Receiving low entropy..\n");
    start_time = clock();
    for(i = 0; i < json_object_get_int(Number_UDP_Packets); i++)
    {
        recvfrom(sockfd, UDPbuffer, BUF_SIZE, 0, ( struct sockaddr *) &client_address, &len); //receive packets into buffer
        packet_id = (int)(((unsigned)UDPbuffer[1] << 8) | UDPbuffer[0]); //reconstruct the packet id
        printf("Retrieved Low Entropy Packet Number: %d\n", packet_id);
    }
    end_time = clock();
    total_time  = (((double)end_time) - ((double)start_time)) / ((double)CLOCKS_PER_SEC);
    low_entropy_time = total_time * 1000;
    printf("Low Entropy Time: %f\n", low_entropy_time);


    printf("Sleeping...\n"); //Sleeping so packets don't get mixed up
    sleep(json_object_get_int(Inter_Measurement_Time));


    //Receive high entropy data
    printf("Receiving high entropy..\n");
    memset(&UDPbuffer, 0, sizeof(UDPbuffer));
    start_time = clock();
    for(i = 0; i < json_object_get_int(Number_UDP_Packets); i++)
    {
        recvfrom(sockfd, UDPbuffer, BUF_SIZE, 0, ( struct sockaddr *) &client_address, &len); //store packets into buffer
        packet_id = (((unsigned)UDPbuffer[1] << 8) | UDPbuffer[0]);
        printf("Retrieved High Entropy Packet Number: %d\n", packet_id);
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
    
    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *) &client_address, sizeof(client_address));
    printf("Message sent\n");
    
    close(sockfd);
    return 0;
}
