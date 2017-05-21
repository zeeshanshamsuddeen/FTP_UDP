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
portno = 5004;


int iSetOption = 1;
/* Create a socket point */
sockfd = socket(AF_INET, SOCK_STREAM, 0);
setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&iSetOption,sizeof(iSetOption));

if (sockfd < 0)
	 {
	  perror("ERROR opening socket");
	  exit(1);
	 }

server = gethostbyname("127.0.0.1");			// returns pointer to corresponding host

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





//username and password check 
char user[256],pass[256],message[256];
int length;



printf("Enter username : ");
bzero(user,256);
//scanf("%s",user);
fgets(user,255,stdin);
length=strlen(user);
user[length-1]='\0';
write(sockfd,user,strlen(user));

bzero(buffer,256);
read(sockfd,buffer,255);

//username is correct
if(strcmp(buffer,"username correct")==0)		
{
	printf("Enter password : ");
	bzero(pass,256);
	//scanf("%s",pass);
	fgets(pass,255,stdin);
	length=strlen(pass);
	pass[length-1]='\0';
	write(sockfd,pass,strlen(pass));
	bzero(message,256);
	read(sockfd,message,255);
	printf("%s",message);
}
else
{
	printf("%s",buffer);		
}


char command[256],command2[256];
char resp[256],op[256],empty[256]="";

if(strcmp(message,"230 Login successful.")==0)
{	
do
{

printf("\nftp> ");
//bzero(command,256);
//scanf("%s",command);
//fgets(command,255,stdin);
fgets(command,255,stdin);
length=strlen(command);
command[length-1]='\0';
//scanf("%[^\n]s",command);
write(sockfd,command,strlen(command));

bzero(resp,256);
read(sockfd,resp,255);
printf("%s",resp);

write(sockfd,empty,1);

bzero(op,256);
read(sockfd,op,255);
printf("%s",op);

}while(strcmp(command,"bye")!=0);

}
return 0;
}
