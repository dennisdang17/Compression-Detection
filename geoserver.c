#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <json-c/json.h>

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	FILE *fp;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
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
	int n;
		if (argc < 2) {
	fprintf(stderr,"ERROR, no port provided\n");
	exit(1);
}
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) 
	error("ERROR opening socket");
	bzero((char *) &serv_addr, sizeof(serv_addr));
		portno = atoi(argv[1]);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
		if (bind(sockfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) < 0) 
				error("ERROR on binding");
				listen(sockfd,5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, 
			(struct sockaddr *) &cli_addr, 
		&clilen);
		if (newsockfd < 0) 
			error("ERROR on accept");

	int ch = 0;
	fp = fopen("myconfig.json","r"); 
	fread(buffer, 1024, 1, fp); //reads files and puts contents inside buffer
	fclose(fp);           
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
	int words;
	read(newsockfd, &words, sizeof(int));
	//printf("Passed integer is : %d\n" , words);      //Ignore , Line for Testing
	while(ch != words)
	{
	read(newsockfd , buffer , 512); 
	fprintf(fp , " %s" , buffer);   
	//printf(" %s %d "  , buffer , ch); //Line for Testing , Ignore
	ch++;
	}
	printf("The file was received successfully\n");
	printf("The new file created is glad5.txt");
	close(newsockfd);
	close(sockfd);
	printf("Server_IP_Address: %s\n", json_object_get_string(Server_IP_Address));
	printf("Source_Port_Number_UDP: %s\n", json_object_get_string(Source_Port_Number_UDP));
	printf("Destination_Port_Number_TCP_Head: %s\n", json_object_get_string(Destination_Port_Number_TCP_Head));
	printf("Destination_Port_Number_TCP_Tail: %s\n", json_object_get_string(Destination_Port_Number_TCP_Tail));
	printf("Port_Number_TCP: %s\n", json_object_get_string(Port_Number_TCP));
	printf("Size_UDP_Payload: %s\n", json_object_get_string(Size_UDP_Payload));
	printf("Inter_Measurement_Time: %s\n", json_object_get_string(Inter_Measurement_Time));
	printf("Number_UDP_Packets: %d\n", json_object_get_int(Number_UDP_Packets));
	printf("TTL_UDP_Packets: %d\n", json_object_get_int(TTL_UDP_Packets));
	return 0; 
}