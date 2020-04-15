#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <ctype.h>

//function to print error
void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main()
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[1024];
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("10.0.0.249");
	serv_addr.sin_port = htons(8765);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
		bzero(buffer,512);

	FILE *f;

	int words = 0;
	char c;
	f=fopen("myconfig.json","r");
	while((c=getc(f))!=EOF) //Counting number of words in the file
		{   
			fscanf(f , "%s" , buffer);
			if(isspace(c)||c=='\t')
				words++;    
		}

	write(sockfd, &words, sizeof(int));
	rewind(f);

	char ch ;
	while(ch != EOF)
		{
			fscanf(f , "%s" , buffer);
			write(sockfd,buffer,512);
				ch = fgetc(f);
		}
	printf("myconfig.json was sent successfully.\n");
	close(sockfd);
		return 0;
}