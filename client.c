#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int main()
{
int sockfd, portno, n;
struct sockaddr_in serv_addr;
struct hostent *server;

char buffer[256];
char temp;
portno = 5001;

/* Create a socket point */
sockfd = socket(AF_INET, SOCK_STREAM, 0);

if (sockfd < 0)
	 {
	  perror("ERROR opening socket");
	  exit(1);
	 }

server = gethostbyname("127.0.0.1");

if (server == NULL) 
	{
	  fprintf(stderr,"ERROR, no such host\n");
	  exit(0);
	}

bzero((char *) &serv_addr, sizeof(serv_addr));
serv_addr.sin_family = AF_INET;
bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
serv_addr.sin_port = htons(portno);

/* Now connect to the server */
if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
	{
	  perror("ERROR connecting");
	  exit(1);
	}

/* Now ask for a message from the user, this message
  * will be read by server
*/
do
	{ 	  	
	   printf("Please enter the message: (for exit, enter E)");
	   bzero(buffer,256);
	   fgets(buffer,255,stdin);
	   temp=buffer[0];		
	   /* Send message to the server */
	   n = write(sockfd, buffer, strlen(buffer));
	   
	   if (n < 0) 
		   {
		      perror("ERROR writing to socket");
		      exit(1);
		   }
		   
	   /* Now read server response */
	   bzero(buffer,256);
	   n = read(sockfd, buffer, 255);
	   
	   if (n < 0) 
		   {
		      perror("ERROR reading from socket");
		      exit(1);
		   }

	   printf("%s\n",buffer);

	}while(temp!='E');

return 0;
}