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
#include <ctype.h>
#include <json-c/json.h>

#define PAYLOAD_SIZE 1000
#define BUF_SIZE 2000

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
    int temp = index;
    printf("Packet ID before: %d\n", temp);
    unsigned char lsb = (unsigned)temp & 0xff;
    unsigned char msb = (unsigned)temp >> 8;
    data[0] = lsb;
    data[1] = msb;
    int packet_id = (int)(((unsigned)data[1] << 8) | data[0]);
    printf("Packet ID after: %d\n", packet_id);
}

void send_file(int sockfd)
{ 
	char buffer[BUF_SIZE]; //stores message to send to server                         
	FILE *fp=fopen("myconfig.json","r"); //opens file called myconfig.json, 'r' reads the file   
	
    while (fgets(buffer,BUF_SIZE,fp) != NULL ) //puts the file into the buffer
	{
        write(sockfd,buffer,strlen(buffer)); //writes to buffer which sends to server  
    }
	
    fclose(fp);
	printf("File was sent successfully.\n");
}

int main(int argc, char * argv[])
{
    int TCPsockfd;
    struct sockaddr_in server_address;
    char buffer[BUF_SIZE];
    struct json_object *parsed_json, *Server_IP_Address, *Source_Port_Number_UDP,
    *Destination_Port_Number_TCP_Head, *Destination_Port_Number_TCP_Tail, *Port_Number_TCP, 
    *Size_UDP_Payload, *Inter_Measurement_Time, *Number_UDP_Packets, *TTL_UDP_Packets;


    if (argv[1] == NULL)
    {
        printf("ERROR!\n Proper ussage ./client 'my_config_file'.json\n");
        return EXIT_FAILURE;
    }
    FILE *fp=fopen(argv[1],"r"); //opens the file myconfig.json
    if(fp == NULL)
    {
        printf("ERROR OPENNING FILE!\n");
        return EXIT_FAILURE;
    }
    fread(buffer, BUF_SIZE, 1, fp); //reads files and puts contents inside buffer
    parsed_json = json_tokener_parse(buffer); //parse JSON file's contents and converts them into a JSON object

    json_object_object_get_ex(parsed_json, "Server_IP_Address", &Server_IP_Address);
    json_object_object_get_ex(parsed_json, "Source_Port_Number_UDP", &Source_Port_Number_UDP);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Head", &Destination_Port_Number_TCP_Head);
    json_object_object_get_ex(parsed_json, "Destination_Port_Number_TCP_Tail", &Destination_Port_Number_TCP_Tail);
    json_object_object_get_ex(parsed_json, "Port_Number_TCP", &Port_Number_TCP);
    json_object_object_get_ex(parsed_json, "Size_UDP_Payload", &Size_UDP_Payload);
    json_object_object_get_ex(parsed_json, "Inter_Measurement_Time", &Inter_Measurement_Time);
    json_object_object_get_ex(parsed_json, "Number_UDP_Packets", &Number_UDP_Packets);
    json_object_object_get_ex(parsed_json, "TTL_UDP_Packets", &TTL_UDP_Packets);


    /*print function that tests wether or not the parsing is successful*/
    printf("Server_IP_Address: %s\n", json_object_get_string(Server_IP_Address));
    printf("Source_Port_Number_UDP: %d\n", json_object_get_int(Source_Port_Number_UDP));
    printf("Destination_Port_Number_TCP_Head: %s\n", json_object_get_string(Destination_Port_Number_TCP_Head));
    printf("Destination_Port_Number_TCP_Tail: %s\n", json_object_get_string(Destination_Port_Number_TCP_Tail));
    printf("Port_Number_TCP: %d\n", json_object_get_int(Port_Number_TCP));
    printf("Size_UDP_Payload: %d\n", json_object_get_int(Size_UDP_Payload));
    printf("Inter_Measurement_Time: %d\n", json_object_get_int(Inter_Measurement_Time));
    printf("Number_UDP_Packets: %d\n", json_object_get_int(Number_UDP_Packets));
    printf("TTL_UDP_Packets: %d\n", json_object_get_int(TTL_UDP_Packets)); //delte after
	
    if ((TCPsockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        printf("Socket Creation Failed.\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("Socket Successfully Created.\n"); 
    }
	memset(&server_address, 0, sizeof(server_address));//zeroes out the server address
	
	server_address.sin_family = AF_INET; // specifies address family with IPv4 Protocol 
    server_address.sin_addr.s_addr = inet_addr(json_object_get_string(Server_IP_Address)); //binds to IP Address
    server_address.sin_port = htons(json_object_get_int(Port_Number_TCP)); //binds to PORT

	// This connects the client socket to server socket 
	if (connect(TCPsockfd, (struct sockaddr *)&server_address, sizeof(server_address)) != 0)
    { 
		printf("Failed to connect to server.\n"); 
		exit(EXIT_FAILURE); 
	} 
	else
	{
        printf("Successfully connected to the server.\n"); 
    }
	
	//calling function to send file
	send_file(TCPsockfd); 
    //closes the socket after transfer
    close(TCPsockfd); 


    //probing phase

    int UDPsockfd, length, DF;
    char datagram[json_object_get_int(Size_UDP_Payload)];
    char compression[25];
    length = sizeof(datagram) / sizeof(char);

    //creating socket
    printf("Creating Socket...\n");
    if ((UDPsockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("socket creation failed\n"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("Socket created successfully!\n");
    }
    
    //Don't fragment bit
    DF = IP_PMTUDISC_DO; //make val equal to dont fragment
    printf("Setting DON'T FRAGMENT bit...\n");
    if(setsockopt(UDPsockfd, IPPROTO_IP, IP_MTU_DISCOVER, &DF, sizeof(DF)) < 0)
    {
        printf("Unable to set DON'T FRAGMENT bit\n");
    }
    else
    {
        printf("DON'T FRAGMENT bit set correctly!\n");
    }

    //Make the packets
    read_low_entropy_data(datagram, length);
    read_high_entropy_data(datagram, length);
    //Probing phase. Sending packet of data
    //send low entropy now
    
    for(int i=0; i < json_object_get_int(Number_UDP_Packets); i++)//chagne to payload size
    {
        set_packet_id(datagram, i);
        sendto(UDPsockfd, datagram, json_object_get_int(Size_UDP_Payload), MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    }
    printf("Low entropy sent!!\n");


    printf("Sleeping...\n");
    sleep(json_object_get_int(Inter_Measurement_Time)); //replace with inter time
    

    //send high entropy now
    for(int i=0; i < json_object_get_int(Number_UDP_Packets); i++) //change to payload size
    {
        set_packet_id(datagram, i);
        sendto(UDPsockfd, datagram, json_object_get_int(Size_UDP_Payload), MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    }
    printf("High entropy sent!!\n");


    //Post Probing

    //receive response
    unsigned int len;
    recvfrom(UDPsockfd, (char *)compression, BUF_SIZE, MSG_WAITALL, (struct sockaddr *) &server_address, &len);  
    printf("Server : %s\n", compression); 
    close(UDPsockfd); 
    return 0;
}
