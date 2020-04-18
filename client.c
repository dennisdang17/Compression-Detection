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

#define BUF_SIZE 2000

//Filling the payload with high entropy data
void read_high_entropy_data(int * data, int len)
{
    FILE * f = fopen("/dev/urandom", "r");
    int temp;
    if(f == NULL)
    {
        return;
    }
    for(int i=2; i < len; i++)
    {
        temp = (int) getc(f);
        data[i] = temp;
    }
}

//Filling payload with low entropy data
void read_low_entropy_data(int * data, int len)
{
    for(int i=2; i < len; i++) 
    {
        data[i] = 0;
    }
}

//Set the packet id
void set_packet_id(int * data, int index)
{
    unsigned int temp = index;
    unsigned char lsb = (unsigned)temp & 0xff; //divide by 255
    unsigned char msb = (unsigned)temp >> 8; // shift bit over by 8
    data[0] = lsb; //set the first index to lower bit
    data[1] = msb; //set the second index to upper bit
}

//Send the file via tcp
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
    int sockfd, DF, i;
    char buffer[BUF_SIZE], compression[25];
    FILE * fp;
    struct sockaddr_in server_address, client_address;
    struct json_object *parsed_json, *Server_IP_Address, *Source_Port_Number_UDP, *Destination_Port_Number_UDP,
    *Destination_Port_Number_TCP_Head, *Destination_Port_Number_TCP_Tail, *Port_Number_TCP, 
    *Size_UDP_Payload, *Inter_Measurement_Time, *Number_UDP_Packets, *TTL_UDP_Packets;

    //Check for proper usage
    if (argv[1] == NULL)
    {
        printf("ERROR!\nProper ussage ./client 'my_config_file'.json\n");
        return EXIT_FAILURE;
    }


    //Pre-Probing Phase TCP


    //Open config file
    fp = fopen(argv[1],"r"); //opens the file myconfig.json
    if(fp == NULL)
    {
        printf("ERROR OPENNING FILE!\n"); //catch null pointer
        return EXIT_FAILURE;
    }
    printf("Parsing...\n");
    fread(buffer, BUF_SIZE, 1, fp); //reads files and puts contents inside buffer
    parsed_json = json_tokener_parse(buffer); //parse JSON file's contents and converts them into a JSON object

    //Store parsed data into variables
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
    printf("Parsing Successful\n");

    //Create tcp connection
    printf("Creating Socket...\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        printf("Socket Creation Failed.\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("Socket Successfully Created.\n"); 
    }

    //Fill in IP header
	memset(&server_address, 0, sizeof(server_address));//zeroes out the server address
    server_address.sin_family = AF_INET; // specifies address family with IPv4 Protocol 
    server_address.sin_addr.s_addr = inet_addr(json_object_get_string(Server_IP_Address)); //binds to IP Address
    server_address.sin_port = htons(json_object_get_int(Port_Number_TCP)); //binds to PORT
    

	// This connects the client socket to server socket 
    printf("Connecting...\n");
	if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) != 0)
    { 
		printf("Failed to connect to server.\n"); 
		exit(EXIT_FAILURE); 
	} 
	else
	{
        printf("Successfully connected to the server.\n"); 
    }
	
	//calling function to send file
	send_file(sockfd); 

    //closes the socket after transfer
    close(sockfd); 



    //PROBING PHASE//

    int datagram[json_object_get_int(Size_UDP_Payload)+2];
    

    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET; // specifies address family with IPv4 Protocol 
    client_address.sin_addr.s_addr = htonl(INADDR_ANY); //binds to IP Address
    client_address.sin_port = htons(json_object_get_int(Source_Port_Number_UDP));
    server_address.sin_port = htons(json_object_get_int(Destination_Port_Number_UDP));

    //Create UDP socket
    printf("Creating Socket...\n");
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    { 
        perror("UDP Socket Creation Failed\n"); 
        exit(EXIT_FAILURE); 
    }
    else
    {
        printf("UDP Socket Creation Successful\n");
    }
    
    //Don't fragment bit
    DF = IP_PMTUDISC_DO; //make val equal to dont fragment
    printf("Setting DON'T FRAGMENT bit...\n");
    if(setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &DF, sizeof(DF)) < 0)
    {
        printf("Unable to set DON'T FRAGMENT bit\n");
    }
    else
    {
        printf("DON'T FRAGMENT bit set correctly!\n");
    }

    if(bind(sockfd, (struct sockaddr *)&client_address, sizeof(client_address)) < 0)
    {
        printf("UDP Socket Bind Failed\n");
        exit(EXIT_FAILURE);
    }

    //Sending data packets
    sleep(5);

    //Low entropy
    read_low_entropy_data(datagram, json_object_get_int(Size_UDP_Payload)+2);
    printf("Sending Low Entropy Data...\n");
    for(i = 1; i < json_object_get_int(Number_UDP_Packets)+1; i++)//chagne to payload size
    {
        set_packet_id(datagram, i);
        sendto(sockfd, datagram, json_object_get_int(Size_UDP_Payload)+2, MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    }
    printf("Low Entropy Sent!\n");

    //Sleeping inter measurement time
    printf("Sleeping...\n");
    sleep(json_object_get_int(Inter_Measurement_Time));

    //High Entropy
    read_high_entropy_data(datagram, json_object_get_int(Size_UDP_Payload)+2);
    printf("Sending High Entropy Data...\n");
    for(i = 1; i < json_object_get_int(Number_UDP_Packets)+1; i++) //change to payload size
    {
        set_packet_id(datagram, i);
        sendto(sockfd, datagram, json_object_get_int(Size_UDP_Payload)+2, MSG_CONFIRM, (const struct sockaddr *) &server_address, sizeof(server_address));
    }
    printf("High entropy sent!\n");


    //Post Probing//
    sleep(5);//let the server catch up


    //TCP socket
    printf("Creating Socket...\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { 
        printf("Socket Creation Failed.\n"); 
        exit(EXIT_FAILURE); 
    } 
    else
    {
        printf("Socket Successfully Created.\n"); 
    }

    //Fill in IP header
	memset(&server_address, 0, sizeof(server_address));//zeroes out the server address
    server_address.sin_family = AF_INET; // specifies address family with IPv4 Protocol 
    server_address.sin_addr.s_addr = inet_addr(json_object_get_string(Server_IP_Address)); //binds to IP Address
    server_address.sin_port = htons(json_object_get_int(Port_Number_TCP)); //binds to PORT
    

	// This connects the client socket to server socket 
    printf("Final Connection...\n");
	if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) != 0)
    { 
		printf("Failed to connect to server.\n"); 
		exit(EXIT_FAILURE); 
	} 
	else
	{
        printf("Successfully connected to the server.\n"); 
    }

    
    recv(sockfd, &compression, sizeof(compression), 0);
    printf("Server's Response: %s\n" , compression);

    close(sockfd); 
    return 0;
}